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

    mutex_.lock();
    int re = avformat_open_input(
        &ic_,
        url,
        nullptr,	// nullptr��ʾ�Զ�ѡ����װ��
        &opts		// �������ã�����rtsp����ʱʱ��
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

    // ��ȡ����Ϣ
    re = avformat_find_stream_info(ic_, nullptr);

    // ��ȡ��ʱ�� ��λ������
    set_total_ms(ic_->duration / (AV_TIME_BASE / 1000));
    cout << "totalMs: " << total_ms() << " ms" << endl;

    // ��ӡ��Ƶ������ϸ��Ϣ
    av_dump_format(ic_, 0, url, 0);

    // ��Ƶ����Ϣ
    video_stream_ = av_find_best_stream(ic_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    AVStream *as = ic_->streams[video_stream_];
    set_width(as->codecpar->width);
    set_height(as->codecpar->height);

    cout << "=====================================================================" << endl;
    cout << video_stream_ << " ��Ƶ��Ϣ" << endl;
    cout << "codec_id: " << as->codecpar->codec_id << endl;
    cout << "format: " << as->codecpar->format << endl;
    cout << "width: " << as->codecpar->width << endl;
    cout << "height: " << as->codecpar->height << endl;
    // ֡�� fps ����ת�������ڷ��գ����Ƿ�ĸΪ0
    cout << "video fps: " << r2d(as->avg_frame_rate) << endl;

    // ��Ƶ����Ϣ
    audio_stream_ = av_find_best_stream(ic_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    as = ic_->streams[audio_stream_];
    set_sample_rate(as->codecpar->sample_rate);
    set_channels(as->codecpar->channels);

    cout << "=====================================================================" << endl;
    cout << audio_stream_ << " ��Ƶ��Ϣ" << endl;
    cout << "codec_id: " << as->codecpar->codec_id << endl;
    cout << "format: " << as->codecpar->format << endl;
    // AVSampleFormat
    cout << "sample_rate: " << as->codecpar->sample_rate << endl;
    cout << "channels: " << as->codecpar->channels << endl;
    // ������Ƶ��˵��һ֡������ʲô��    ����ŵ��ǣ���ͨ����һ������������
    cout << "frame_size: " << as->codecpar->frame_size << endl;
    // 1024 * 2 * 2 = 4096        fps = sample_rate / frame_size
    mutex_.unlock();

    return true;
}

AVPacket *XDemux::Read()
{
    mutex_.lock();
    // �ݴ�
    if (!ic_)
    {
        mutex_.unlock();
        return nullptr;
    }
    AVPacket *pkt = av_packet_alloc();
    // ��ȡһ֡��������ռ�
    int re = av_read_frame(ic_, pkt);
    if (re != 0)
    {
        mutex_.unlock();
        av_packet_free(&pkt);
        return nullptr;
    }
    // ptsת��Ϊ����
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
    // ��ֹ����
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
    // �����ȡ����
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
    // �����ȡ����
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
    // �����ȡ����
    avformat_close_input(&ic_);
    // ý����ʱ����Ϊ0
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

