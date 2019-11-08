#ifndef XDECODE_THREAD_H
#define XDECODE_THREAD_H

#include <QThread>
#include <mutex>
#include <list>


struct AVCodecParameters;
struct AVPacket;
class XDecode;
class XDecodeThread : public QThread
{
public:
    XDecodeThread();
    virtual ~XDecodeThread();
    // 除了清理资源，还停止线程
    virtual void Close();
    // 清理队列
    virtual void Clear();
    virtual void Push(AVPacket *pkt);
    // 取出一帧数据，并出栈，如果没有数据，返回nullptr
    virtual AVPacket *Pop();

public:
    // 最大队列
    int maxList = 100;
    bool isExit = false;

protected:
    std::list<AVPacket *> packs_;
    std::mutex mutex_;
    XDecode *decode_ = nullptr;
};

#endif // !XDECODE_THREAD_H

