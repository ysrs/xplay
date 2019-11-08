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

    mutex_.lock();
    int re = avformat_open_input(
        &ic_,
        url,
        nullptr,	// nullptr表示自动选择解封装器
        &opts		// 参数设置，比如rtsp的延时时间
    );

    if (re != 0)
    {
        mutex_.unlock();
        char buf[1024] = { 0 };
        av_strerror(re, buf, sizeof(buf) - 1);
        cout << "open " << url << " failed! :" << buf << endl;
        return false;
    }
    cout << "open " << url << " success!" << endl;

    // 获取流信息
    re = avformat_find_stream_info(ic_, nullptr);

    // 获取总时长 单位：毫秒
    set_total_ms(ic_->duration / (AV_TIME_BASE / 1000));
    cout << "totalMs: " << total_ms() << " ms" << endl;

    // 打印视频流的详细信息
    av_dump_format(ic_, 0, url, 0);

    // 视频流信息
    video_stream_ = av_find_best_stream(ic_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream *as = ic_->streams[video_stream_];
    set_width(as->codecpar->width);
    set_height(as->codecpar->height);

    cout << "=====================================================================" << endl;
    cout << video_stream_ << " 视频信息" << endl;
    cout << "codec_id: " << as->codecpar->codec_id << endl;
    cout << "format: " << as->codecpar->format << endl;
    cout << "width: " << as->codecpar->width << endl;
    cout << "height: " << as->codecpar->height << endl;
    // 帧率 fps 分数转换，存在风险，就是分母为0
    cout << "video fps: " << r2d(as->avg_frame_rate) << endl;

    // 音频流信息
    audio_stream_ = av_find_best_stream(ic_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    as = ic_->streams[audio_stream_];
    set_sample_rate(as->codecpar->sample_rate);
    set_channels(as->codecpar->channels);

    cout << "=====================================================================" << endl;
    cout << audio_stream_ << " 音频信息" << endl;
    cout << "codec_id: " << as->codecpar->codec_id << endl;
    cout << "format: " << as->codecpar->format << endl;
    // AVSampleFormat
    cout << "sample_rate: " << as->codecpar->sample_rate << endl;
    cout << "channels: " << as->codecpar->channels << endl;
    // 对于音频来说，一帧数据是什么？    它存放的是（单通道）一定量的样本数
    cout << "frame_size: " << as->codecpar->frame_size << endl;
    // 1024 * 2 * 2 = 4096        fps = sample_rate / frame_size
    mutex_.unlock();

    return true;
}

AVPacket *XDemux::Read()
{
    mutex_.lock();
    // 容错
    if (!ic_)
    {
        mutex_.unlock();
        return nullptr;
    }
    AVPacket *pkt = av_packet_alloc();
    // 读取一帧，并分配空间
    int re = av_read_frame(ic_, pkt);
    if (re != 0)
    {
        mutex_.unlock();
        av_packet_free(&pkt);
        return nullptr;
    }
    // pts转换为毫秒
    pkt->pts = pkt->pts * (1000 * (r2d(ic_->streams[pkt->stream_index]->time_base)));
    pkt->dts = pkt->dts * (1000 * (r2d(ic_->streams[pkt->stream_index]->time_base)));
    //cout << pkt->pts << " " << flush;

    mutex_.unlock();

    return pkt;
}

AVPacket *XDemux::ReadVideo()
{
    mutex_.lock();
    if (!ic_)
    {
        mutex_.unlock();
        return nullptr;
    }
    mutex_.unlock();

    AVPacket *pkt = nullptr;
    // 防止阻塞
    for (int i = 0; i < 20; ++i)
    {
        pkt = Read();
        if (!pkt)
        {
            break;
        }
        if (pkt->stream_index == video_stream_)
        {
            break;
        }
        av_packet_free(&pkt);
    }

    return pkt;
}

AVCodecParameters *XDemux::CopyVPara()
{
    mutex_.lock();
    if (!ic_)
    {
        mutex_.unlock();
        return nullptr;
    }
    AVCodecParameters *pa = avcodec_parameters_alloc();
    avcodec_parameters_copy(pa, ic_->streams[video_stream_]->codecpar);
    mutex_.unlock();

    return pa;
}

AVCodecParameters *XDemux::CopyAPara()
{
    mutex_.lock();
    if (!ic_)
    {
        mutex_.unlock();
        return nullptr;
    }
    AVCodecParameters *pa = avcodec_parameters_alloc();
    avcodec_parameters_copy(pa, ic_->streams[audio_stream_]->codecpar);
    mutex_.unlock();

    return pa;
}

bool XDemux::Seek(double pos)
{
    mutex_.lock();
    if (!ic_)
    {
        mutex_.unlock();
        return false;
    }
    // 清理读取缓存
    avformat_flush(ic_);
    long long seekPos = ic_->streams[video_stream_]->duration * pos;
    //long long pos = (double)ms / (double)(1000 * r2d(ic->streams[pkt->stream_index]->time_base));
    int re = av_seek_frame(ic_, video_stream_, seekPos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    mutex_.unlock();
    if (re < 0)
    {
        return false;
    }

    return true;
}

void XDemux::Clear()
{
    mutex_.lock();
    if (!ic_)
    {
        mutex_.unlock();
        return;
    }
    // 清理读取缓存
    avformat_flush(ic_);
    mutex_.unlock();
}

void XDemux::Close()
{
    mutex_.lock();
    if (!ic_)
    {
        mutex_.unlock();
        return;
    }
    // 清理读取缓存
    avformat_close_input(&ic_);
    // 媒体总时长置为0
    set_total_ms(0);
    mutex_.unlock();
}

bool XDemux::IsAudio(AVPacket *pkt)
{
    if (!pkt)
    {
        return false;
    }

    if (pkt->stream_index == video_stream_)
    {
        return false;
    }

    return true;
}

