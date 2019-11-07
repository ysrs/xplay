#ifndef XAUDIO_THREAD_H
#define XAUDIO_THREAD_H

#include <QThread>
#include <mutex>
#include <list>

#include "xdecode_thread.h"


class XAudioPlay;
class XResample;
class XAudioThread : public XDecodeThread
{
public:
	XAudioThread();
	virtual ~XAudioThread();

	void run() override;
	// 打开，不管成功与否，都清理para空间
	virtual bool Open(AVCodecParameters *para, int sampleRate, int channels);
	// 停止线程，清理资源
	void Close() override;
	void Clear() override;

	void SetPause(bool isPause);

public:
	// 当前音频播放的pts
	long long pts = 0;
	bool isPause = false;

protected:
	std::mutex amux;
	XAudioPlay *ap = nullptr;
	XResample *res = nullptr;
};

#endif // !XAUDIO_THREAD_H

