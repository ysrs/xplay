#ifndef XAUDIO_PLAY_H
#define XAUDIO_PLAY_H


class XAudioPlay
{
public:
	XAudioPlay();
	virtual ~XAudioPlay();

	static XAudioPlay *Get();
	// 打开音频播放
	virtual bool Open() = 0;
	// 关闭音频播放
	virtual bool Close() = 0;
	virtual void Clear() = 0;

	// 播放音频
	virtual bool Write(const unsigned char *data, int datasize) = 0;
	virtual int GetFree() = 0;
	// 返回缓冲中还没有播放的时间，单位：ms
	virtual long long GetNoPlayMs() = 0;

	virtual void SetPause(bool isPause) = 0;

public:
	int sampleRate = 44100;
	int sampleSize = 16;
	int channels = 2;
};

#endif // !XAUDIO_PLAY_H

