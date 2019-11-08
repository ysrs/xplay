#ifndef XVIDEO_THREAD_H
#define XVIDEO_THREAD_H

#include <mutex>
#include <list>

#include "ivideo_call.h"
#include "xdecode_thread.h"


// �������ʾ��Ƶ
class XVideoThread : public XDecodeThread
{
public:
    XVideoThread();
    virtual ~XVideoThread();

    void run() override;
    // �򿪣����ܳɹ���񣬶�����para�ռ�
    virtual bool Open(AVCodecParameters *para, IVideoCall *call, int width, int height);
    // ����pts��������յ��Ľ�������pts ��= seekpts��return true��������ʾ����
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
    // ͬ��ʱ�����ⲿ����
    long long syn_pts_ = 0;
    bool pause_ = false;
};

#endif // !XVIDEO_THREAD_H

