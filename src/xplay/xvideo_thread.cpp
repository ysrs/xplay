#include "xvideo_thread.h"
#include "XDecode.h"

#include <iostream>


using namespace std;

XVideoThread::XVideoThread()
{
}


XVideoThread::~XVideoThread()
{
}


void XVideoThread::run()
{
    while (!isExit)
    {
        vmux.lock();
        if (pause_)
        {
            vmux.unlock();
            msleep(5);
            continue;
        }

        // 音视频同步
        if ((syn_pts_ > 0) && (syn_pts_ < decode_->pts()))
        {
            vmux.unlock();
            msleep(1);
            continue;
        }

        // 没有数据
        //if (packs.empty() || !decode)
        //{
        //	vmux.unlock();
        //	msleep(1);
        //	continue;
        //}
        //
        //AVPacket *pkt = packs.front();
        //packs.pop_front();

        AVPacket *pkt = Pop();


        bool re = decode_->Send(pkt);
        if (!re)
        {
            vmux.unlock();
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
            // 显示视频
            if (call)
            {
                call->Repaint(frame);
            }
        }

        vmux.unlock();
    }
}

bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
    if (!para)
    {
        return false;
    }

    Clear();

    vmux.lock();
    syn_pts_ = 0;
    // 初始化显示窗口
    this->call = call;
    if (call)
    {
        call->Init(width, height);
    }
    vmux.unlock();

    bool re = true;
    if (!decode_->Open(para))
    {
        cout << "video XDecode open failed!" << endl;
        re = false;
    }

    cout << "XVideoThread::Open() re: " << re << endl;

    return re;
}

bool XVideoThread::RepaintPts(AVPacket *pkt, long long seekpts)
{
    vmux.lock();

    bool re = decode_->Send(pkt);
    if (!re)
    {
        // 表示结束解码
        vmux.unlock();
        return true;
    }
    AVFrame *frame = decode_->Recv();
    if (!frame)
    {
        vmux.unlock();
        return false;
    }
    // 到达位置
    if (decode_->pts() >= seekpts)
    {
        if (call)
        {
            call->Repaint(frame);
        }
        vmux.unlock();
        return true;
    }
    XFreeFrame(&frame);
    vmux.unlock();

    return false;
}

void XVideoThread::SetPause(bool isPause)
{
    vmux.lock();
    this->pause_ = isPause;
    vmux.unlock();
}

