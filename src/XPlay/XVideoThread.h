#pragma once
#include <mutex>
#include <list>

#include "IVideoCall.h"
#include "XDecodeThread.h"


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

public:
	// 同步时间由外部传入
	long long synpts = 0;
	bool isPause = false;

protected:
	std::mutex vmux;
	IVideoCall *call = nullptr;
};

