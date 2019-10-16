#pragma once
#include <mutex>
#include <list>

#include "IVideoCall.h"
#include "XDecodeThread.h"


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

public:
	// ͬ��ʱ�����ⲿ����
	long long synpts = 0;
	bool isPause = false;

protected:
	std::mutex vmux;
	IVideoCall *call = nullptr;
};

