#include "xdecode_thread.h"
#include "xdecode.h"


XDecodeThread::XDecodeThread()
{
    // 打开解码器
    if (!decode_)
    {
        decode_ = new XDecode();
    }
}


XDecodeThread::~XDecodeThread()
{
    // 等待线程退出
    isExit = true;
    wait();
}


void XDecodeThread::Close()
{
    Clear();

    // 等待线程退出
    isExit = true;
    wait();

    mutex_.lock();
    decode_->Close();
    delete decode_;
    decode_ = nullptr;
    mutex_.unlock();
}

void XDecodeThread::Clear()
{
    mutex_.lock();
    decode_->Clear();
    while (!packs_.empty())
    {
        AVPacket *pkt = packs_.front();
        XFreePacket(&pkt);
        packs_.pop_front();
    }
    mutex_.unlock();
}

void XDecodeThread::Push(AVPacket *pkt)
{
    if (!pkt)
    {
        return;
    }

    // 阻塞
    while (!isExit)
    {
        mutex_.lock();
        if (packs_.size() < maxList)
        {
            packs_.push_back(pkt);
            mutex_.unlock();
            break;
        }
        mutex_.unlock();
        msleep(1);
    }
}

AVPacket *XDecodeThread::Pop()
{
    mutex_.lock();

    if (packs_.empty())
    {
        mutex_.unlock();
        return nullptr;
    }
    AVPacket *pkt = packs_.front();
    packs_.pop_front();
    mutex_.unlock();

    return pkt;
}

