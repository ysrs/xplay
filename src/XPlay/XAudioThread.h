#pragma once
#include <QThread>
#include <mutex>
#include <list>

#include "XDecodeThread.h"


class XAudioPlay;
class XResample;

class XAudioThread : public XDecodeThread
{
public:
	XAudioThread();
	virtual ~XAudioThread();

	void run() override;
	// �򿪣����ܳɹ���񣬶�����para�ռ�
	virtual bool Open(AVCodecParameters *para, int sampleRate, int channels);
	// ֹͣ�̣߳�������Դ
	void Close() override;
	void Clear() override;

	void SetPause(bool isPause);

public:
	// ��ǰ��Ƶ���ŵ�pts
	long long pts = 0;
	bool isPause = false;

protected:
	std::mutex amux;
	XAudioPlay *ap = nullptr;
	XResample *res = nullptr;
};

