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
		// ��ʼ����װ��
		av_register_all();

		// ��ʼ������⣨���Դ�rtsp rtmp httpЭ�����ý����Ƶ��
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
	// ��������
	AVDictionary *opts = nullptr;
	// ����rtsp����tcpЭ���
	av_dict_set(&opts, "rtsp_transport", "tcp", 0);
	// ����������ʱ
	av_dict_set(&opts, "max_delay", "500", 0);

	mux.lock();
	int re = avformat_open_input(
		&ic,
		url,
		nullptr,	// nullptr��ʾ�Զ�ѡ����װ��
		&opts		// �������ã�����rtsp����ʱʱ��
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

	// ��ȡ����Ϣ
	re = avformat_find_stream_info(ic, nullptr);

	// ��ȡ��ʱ�� ��λ������
	totalMs = ic->duration / (AV_TIME_BASE / 1000);
	cout << "totalMs: " << totalMs << " ms" << endl;

	// ��ӡ��Ƶ������ϸ��Ϣ
	av_dump_format(ic, 0, url, 0);

	// ��Ƶ����Ϣ
	videoStream = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	AVStream *as = ic->streams[videoStream];
	width = as->codecpar->width;
	height = as->codecpar->height;

	cout << "=====================================================================" << endl;
	cout << videoStream << " ��Ƶ��Ϣ" << endl;
	cout << "codec_id: " << as->codecpar->codec_id << endl;
	cout << "format: " << as->codecpar->format << endl;
	cout << "width: " << as->codecpar->width << endl;
	cout << "height: " << as->codecpar->height << endl;
	// ֡�� fps ����ת�������ڷ��գ����Ƿ�ĸΪ0
	cout << "video fps: " << r2d(as->avg_frame_rate) << endl;

	// ��Ƶ����Ϣ
	audioStream = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	as = ic->streams[audioStream];
	sampleRate = as->codecpar->sample_rate;
	channels = as->codecpar->channels;

	cout << "=====================================================================" << endl;
	cout << audioStream << " ��Ƶ��Ϣ" << endl;
	cout << "codec_id: " << as->codecpar->codec_id << endl;
	cout << "format: " << as->codecpar->format << endl;
	// AVSampleFormat
	cout << "sample_rate: " << as->codecpar->sample_rate << endl;
	cout << "channels: " << as->codecpar->channels << endl;
	// ������Ƶ��˵��һ֡������ʲô��    ����ŵ��ǣ���ͨ����һ������������
	cout << "frame_size: " << as->codecpar->frame_size << endl;
	// 1024 * 2 * 2 = 4096        fps = sample_rate / frame_size
	mux.unlock();

	return true;
}

AVPacket *XDemux::Read()
{
	mux.lock();
	// �ݴ�
	if (!ic)
	{
		mux.unlock();
		return nullptr;
	}
	AVPacket *pkt = av_packet_alloc();
	// ��ȡһ֡��������ռ�
	int re = av_read_frame(ic, pkt);
	if (re != 0)
	{
		mux.unlock();
		av_packet_free(&pkt);
		return nullptr;
	}
	// ptsת��Ϊ����
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
	// ��ֹ����
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
	// �����ȡ����
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
	// �����ȡ����
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
	// �����ȡ����
	avformat_close_input(&ic);
	// ý����ʱ����Ϊ0
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

