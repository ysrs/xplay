#ifndef XAUDIO_PLAY_H
#define XAUDIO_PLAY_H


class XAudioPlay
{
public:
	XAudioPlay();
	virtual ~XAudioPlay();

	static XAudioPlay *Get();
	// ����Ƶ����
	virtual bool Open() = 0;
	// �ر���Ƶ����
	virtual bool Close() = 0;
	virtual void Clear() = 0;

	// ������Ƶ
	virtual bool Write(const unsigned char *data, int datasize) = 0;
	virtual int GetFree() = 0;
	// ���ػ����л�û�в��ŵ�ʱ�䣬��λ��ms
	virtual long long GetNoPlayMs() = 0;

	virtual void SetPause(bool isPause) = 0;

public:
	int sampleRate = 44100;
	int sampleSize = 16;
	int channels = 2;
};

#endif // !XAUDIO_PLAY_H

