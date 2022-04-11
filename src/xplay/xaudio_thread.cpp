#include "xaudio_thread.h"
#include "xdecode.h"
#include "xaudio_play.h"
#include "xresample.h"

#include <iostream>


using namespace std;
XAudioThread::XAudioThread()
{
    if (!resample_)
    {
        resample_ = new XResample();
    }
    if (!audio_play_)
    {
        audio_play_ = XAudioPlay::Get();
    }
}


XAudioThread::~XAudioThread()
{
    // 等待线程退出
    isExit = true;
    wait();
}


void XAudioThread::run()
{
    unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
    while (!isExit)
    {
        audio_mtx_.lock();
        if (isPause)
        {
            audio_mtx_.unlock();
            msleep(5);
            continue;
        }

        AVPacket *pkt = Pop();
        bool re = decode_->Send(pkt);
        if (!re)
        {
            audio_mtx_.unlock();
            msleep(1);
            continue;;
        }
        // 一次Send，多次Recv
        while (!isExit)
        {
            AVFrame *frame = decode_->Recv();
            if (!frame)
            {
                break;
            }
            // 减去缓冲中未播放的时间，单位：ms
            pts = decode_->pts() - audio_play_->GetNoPlayMs();

            //cout << "audio pts = " << pts << endl;
            // 重采样
            int size = resample_->Resample(frame, pcm);

            // 播放音频
            while (!isExit)
            {
                if (size <= 0)
                {
                    break;
                }
                // 缓冲未播完，空间不够
                if (audio_play_->GetFree() < size || isPause)
                {
                    msleep(1);
                    continue;
                }
                audio_play_->Write(pcm, size);
                break;
            }
        }

        audio_mtx_.unlock();
    }

    delete[] pcm;
}

bool XAudioThread::Open(AVCodecParameters *para, int sampleRate, int channels)
{
    if (!para)
    {
        return false;
    }

    Clear();

    audio_mtx_.lock();
    pts = 0;
    bool re = true;
    if (!resample_->Open(para, false))
    {
        cout << "XAudioPlay open failed!" << endl;
        re = false;
    }

    audio_play_->set_sample_rate(sampleRate);
    audio_play_->set_channels(channels);
    if (!audio_play_->Open())
    {
        cout << "XAudioPlay open failed!" << endl;
        re = false;
    }

    if (!decode_->Open(para))
    {
        cout << "audio XDecode open failed!" << endl;
        re = false;
    }

    audio_mtx_.unlock();
    cout << "XAudioThread::Open() re: " << re << endl;

    return re;
}

void XAudioThread::Close()
{
    XDecodeThread::Close();
    if (resample_)
    {
        resample_->Close();

        audio_mtx_.lock();
        delete resample_;
        resample_ = nullptr;
        audio_mtx_.unlock();
    }

    if (audio_play_)
    {
        audio_play_->Close();

        audio_mtx_.lock();
        audio_play_ = nullptr;
        audio_mtx_.unlock();
    }
}

void XAudioThread::Clear()
{
    XDecodeThread::Clear();

    mutex_.lock();
    if (audio_play_)
    {
        audio_play_->Clear();
    }
    mutex_.unlock();
}

void XAudioThread::SetPause(bool isPause)
{
    //audio_mtx_.lock();
    this->isPause = isPause;
    if (audio_play_)
    {
        audio_play_->SetPause(isPause);
    }
    //audio_mtx_.unlock();
}

