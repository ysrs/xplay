#ifndef XDEMUX_THREAD_H
#define XDEMUX_THREAD_H

#include <QThread>
#include <mutex>

#include "ivideo_call.h"


class XDemux;
class XVideoThread;
class XAudioThread;

class XDemuxThread : public QThread
{
public:
    XDemuxThread();
    virtual ~XDemuxThread();
    // �������󲢴�
    virtual bool Open(const char *url, IVideoCall *call);
    // ���������߳�
    virtual void Start();
    // �ر��̣߳�������Դ
    virtual void Close();
    virtual void Clear();
    virtual void Seek(double pos);
    void run() override;

    void SetPause(bool isPause);

    long long pts()
    {
        return pts_;
    }

    long long total_ms()
    {
        return total_ms_;
    }

    bool pause()
    {
        return pause_;
    }

protected:
    std::mutex mutex_;
    XDemux *demux_ = nullptr;
    XVideoThread *video_thread_ = nullptr;
    XAudioThread *audio_thread_ = nullptr;

private:
    bool exit_ = false;
    long long pts_ = 0;
    long long total_ms_ = 0;
    bool pause_ = false;
};

#endif // !XDEMUX_THREAD_H

