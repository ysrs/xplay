#pragma once
#include <QThread>
#include <mutex>

#include "IVideoCall.h"


class XDemux;
class XVideoThread;
class XAudioThread;

class XDemuxThread : public QThread
{
public:
	XDemuxThread();
	virtual ~XDemuxThread();
	// 创建对象并打开
	virtual bool Open(const char *url, IVideoCall *call);
	// 启动所有线程
	virtual void Start();
	// 关闭线程，清理资源
	virtual void Close();
	virtual void Clear();
	virtual void Seek(double pos);
	void run() override;

	void SetPause(bool isPause);
	

public:
	bool isExit = false;
	long long pts = 0;
	long long totalMs = 0;
	bool isPause = false;

protected:
	std::mutex mux;
	XDemux *demux = nullptr;
	XVideoThread *vt = nullptr;
	XAudioThread *at = nullptr;
};

