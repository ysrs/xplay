#ifndef XVIDEO_THREAD_H
#define XVIDEO_THREAD_H

#include <mutex>
#include <list>

#include "ivideo_call.h"
#include "xdecode_thread.h"


// 解码和显示视频
class XVideoThread : public XDecodeThread
{
public:
    XVideoThread();
    virtual ~XVideoThread();

    void run() override;
    // 打开，不管成功与否，都清理para空间
    virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
    // 解码pts，如果接收到的解码数据pts 》= seekpts，return true，并且显示画面
    virtual bool RepaintPts(AVPacket *pkt, long long seekpts);
    void SetPause(bool isPause);

    void set_syn_pts(long long syn_pts)
    {
        syn_pts_ = syn_pts;
    }
    long long syn_pts()
    {
        return syn_pts_;
    }

protected:
    std::mutex vmux;
    IVideoCall *call = nullptr;

private:
    // 同步时间由外部传入
    long long syn_pts_ = 0;
    bool pause_ = false;
};

#endif // !XVIDEO_THREAD_H

