#include "xdemux_thread.h"
#include "xdemux.h"
#include "XDecode.h"
#include "xvideo_thread.h"
#include "xaudio_thread.h"

#include <iostream>


using namespace std;

XDemuxThread::XDemuxThread()
{
}


XDemuxThread::~XDemuxThread()
{
    exit_ = true;
    wait();
}


bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
    if (nullptr == url || '\0' == url[0])
    {
        return false;
    }

    mutex_.lock();

    // �򿪽��װ
    bool re = demux_->Open(url);
    if (!re)
    {
        cout << "demux->Open(url) failed!" << endl;
        return false;
    }
    // ����Ƶ�������ʹ����߳�
    if (!video_thread_->Open(demux_->CopyVPara(), call, demux_->width(), demux_->height()))
    {
        re = false;
        cout << "vt->Open() failed!" << endl;
    }
    // ����Ƶ�������ʹ����߳�
    if (!audio_thread_->Open(demux_->CopyAPara(), demux_->sample_rate(), demux_->channels()))
    {
        re = false;
        cout << "at->Open() failed!" << endl;
    }
    total_ms_ = demux_->total_ms();
    mutex_.unlock();
    cout << "XDemuxThread::Open() re: " << re << endl;

    return re;
}

void XDemuxThread::Start()
{
    mutex_.lock();
    if (!demux_)
    {
        demux_ = new XDemux();
    }
    if (!video_thread_)
    {
        video_thread_ = new XVideoThread();
    }
    if (!audio_thread_)
    {
        audio_thread_ = new XAudioThread();
    }

    // ������ǰ�߳�
    QThread::start();

    if (video_thread_)
    {
        video_thread_->start();
    }
    if (audio_thread_)
    {
        audio_thread_->start();
    }

    mutex_.unlock();
}

void XDemuxThread::Close()
{
    exit_ = true;
    wait();

    if (video_thread_)
    {
        video_thread_->Close();

        mutex_.lock();
        delete video_thread_;
        video_thread_ = nullptr;
        mutex_.unlock();
    }
    if (audio_thread_)
    {
        audio_thread_->Close();

        mutex_.lock();
        delete audio_thread_;
        audio_thread_ = nullptr;
        mutex_.unlock();
    }
}

void XDemuxThread::Clear()
{
    mutex_.lock();
    if (demux_)
    {
        demux_->Clear();
    }
    if (video_thread_)
    {
        video_thread_->Clear();
    }
    if (audio_thread_)
    {
        audio_thread_->Clear();
    }
    mutex_.unlock();
}

void XDemuxThread::Seek(double pos)
{
    // ������
    Clear();

    mutex_.lock();
    bool status = this->pause_;
    mutex_.unlock();

    // ��ͣ
    SetPause(true);

    mutex_.lock();
    if (demux_)
    {
        demux_->Seek(pos);
    }

    // ʵ��Ҫ��ʾ��λ��
    long long seekPso = pos * demux_->total_ms();
    while (!exit_)
    {
        AVPacket *pkt = demux_->ReadVideo();
        if (!pkt)
        {
            break;
        }
        // ������뵽seekpts
        if (video_thread_->RepaintPts(pkt, seekPso))
        {
            pts_ = seekPso;
            break;
        }
    }
    mutex_.unlock();

    // ���seekʱ�Ƿ���ͣ״̬����ô���ͻָ��ɲ���
    if (!status)
    {
        SetPause(false);
    }
}

void XDemuxThread::run()
{
    while (!exit_)
    {
        mutex_.lock();
        if (pause_)
        {
            mutex_.unlock();
            msleep(5);
            continue;
        }

        if (!demux_)
        {
            mutex_.unlock();
            msleep(5);
            continue;
        }
        // ����Ƶͬ��
        if (video_thread_ && audio_thread_)
        {
            pts_ = audio_thread_->pts;
            video_thread_->set_syn_pts(audio_thread_->pts);
        }

        AVPacket *pkt = demux_->Read();
        if (!pkt)
        {
            mutex_.unlock();
            msleep(5);
            continue;
        }

        // �ж���������Ƶ
        if (demux_->IsAudio(pkt))
        {
            if (audio_thread_)
            {
                audio_thread_->Push(pkt);
            }
        }
        else
        {
            // ��Ƶ
            if (video_thread_)
            {
                video_thread_->Push(pkt);
            }
        }

        mutex_.unlock();
        msleep(1);
    }
}

void XDemuxThread::SetPause(bool isPause)
{
    mutex_.lock();
    this->pause_ = isPause;
    if (audio_thread_)
    {
        audio_thread_->SetPause(isPause);
    }
    if (video_thread_)
    {
        video_thread_->SetPause(isPause);
    }
    mutex_.unlock();
}

