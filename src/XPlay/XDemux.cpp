#include "XDemux.h"
#include <iostream>

extern "C"
{
#include <libavformat/avformat.h>
}

#ifdef _WIN32
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#endif

using namespace std;

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

XDemux::XDemux()
{
	static  bool isFirst = true;
	static std::mutex dmux;
	dmux.lock();

	if (isFirst)
	{
		// 初始化封装库
		av_register_all();

		// 初始化网络库（可以打开rtsp rtmp http协议的流媒体视频）
		avformat_network_init();

		isFirst = false;
	}

	dmux.unlock();
}


XDemux::~XDemux()
{
}


bool XDemux::Open(const char *url)
{
	Close();
	// 参数设置
	AVDictionary *opts = nullptr;
	// 设置rtsp流以tcp协议打开
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	// 设置网络延时
	av_dict_set(&opts, "max_delay", "500", 0);

	mux.lock();
	int re = avformat_open_input(
		&ic,
		url,
		nullptr,	// nullptr表示自动选择解封装器
		&opts		// 参数设置，比如rtsp的延时时间
	);

	if (re != 0)
	{
		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "open " << url << " failed! :" << buf << endl;
		return false;
	}
	cout << "open " << url << " success!" << endl;

	// 获取流信息
	re = avformat_find_stream_info(ic, nullptr);

	// 获取总时长 单位：毫秒
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs: " << totalMs << " ms" << endl;

	// 打印视频流的详细信息
	av_dump_format(ic, 0, url, 0);

	// 视频流信息
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	AVStream *as = ic->streams[videoStream];
	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "=====================================================================" << endl;
	cout << videoStream << " 视频信息" << endl;
	cout << "codec_id: " << as->codecpar->codec_id << endl;
	cout << "format: " << as->codecpar->format << endl;
	cout << "width: " << as->codecpar->width << endl;
	cout << "height: " << as->codecpar->height << endl;
	// 帧率 fps 分数转换，存在风险，就是分母为0
	cout << "video fps: " << r2d(as->avg_frame_rate) << endl;

	// 音频流信息
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	as = ic->streams[audioStream];
	sampleRate = as->codecpar->sample_rate;
	channels = as->codecpar->channels;

	cout << "=====================================================================" << endl;
	cout << audioStream << " 音频信息" << endl;
	cout << "codec_id: " << as->codecpar->codec_id << endl;
	cout << "format: " << as->codecpar->format << endl;
	// AVSampleFormat
	cout << "sample_rate: " << as->codecpar->sample_rate << endl;
	cout << "channels: " << as->codecpar->channels << endl;
	// 对于音频来说，一帧数据是什么？    它存放的是（单通道）一定量的样本数
	cout << "frame_size: " << as->codecpar->frame_size << endl;
	// 1024 * 2 * 2 = 4096        fps = sample_rate / frame_size
	mux.unlock();

	return true;
}

AVPacket *XDemux::Read()
{
	mux.lock();
	// 容错
	if (!ic)
	{
		mux.unlock();
		return nullptr;
	}
	AVPacket *pkt = av_packet_alloc();
	// 读取一帧，并分配空间
	int re = av_read_frame(ic, pkt);
	if (re != 0)
	{
		mux.unlock();
		av_packet_free(&pkt);
		return nullptr;
	}
	// pts转换为毫秒
	pkt->pts = pkt->pts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	pkt->dts = pkt->dts * (1000 * (r2d(ic->streams[pkt->stream_index]->time_base)));
	//cout << pkt->pts << " " << flush;

	mux.unlock();

	return pkt;
}

AVPacket *XDemux::ReadVideo()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return nullptr;
	}
	mux.unlock();

	AVPacket *pkt = nullptr;
	// 防止阻塞
	for (int i=0; i<20; ++i)
	{
		pkt = Read();
		if (!pkt)
		{
			break;
		}
		if (pkt->stream_index == videoStream)
		{
			break;
		}
		av_packet_free(&pkt);
	}

	return pkt;
}

AVCodecParameters *XDemux::CopyVPara()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return nullptr;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[videoStream]->codecpar);
	mux.unlock();

	return pa;
}

AVCodecParameters *XDemux::CopyAPara()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return nullptr;
	}
	AVCodecParameters *pa = avcodec_parameters_alloc();
	avcodec_parameters_copy(pa, ic->streams[audioStream]->codecpar);
	mux.unlock();

	return pa;
}

bool XDemux::Seek(double pos)
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return false;
	}
	// 清理读取缓存
	avformat_flush(ic);
	long long seekPos = ic->streams[videoStream]->duration * pos;
	//long long pos = (double)ms / (double)(1000 * r2d(ic->streams[pkt->stream_index]->time_base));
	int re = av_seek_frame(ic, videoStream, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
	mux.unlock();
	if (re < 0)
	{
		return false;
	}

	return true;
}

void XDemux::Clear()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	// 清理读取缓存
	avformat_flush(ic);
	mux.unlock();
}

void XDemux::Close()
{
	mux.lock();
	if (!ic)
	{
		mux.unlock();
		return;
	}
	// 清理读取缓存
	avformat_close_input(&ic);
	// 媒体总时长置为0
	totalMs = 0;
	mux.unlock();
}

bool XDemux::IsAudio(AVPacket *pkt)
{
	if (!pkt)
	{
		return false;
	}

	if (pkt->stream_index == videoStream)
	{
		return false;
	}
	
	return true;
}

