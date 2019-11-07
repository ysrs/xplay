#include "xaudio_play.h"
#include <QAudioFormat>
#include <QAudioOutput>
#include <mutex>


class CXAudioPlay : public XAudioPlay
{
public:
	bool Open() override
	{
		Close();
		QAudioFormat fmt;
		fmt.setSampleRate(sampleRate);
		fmt.setSampleSize(sampleSize);
		fmt.setChannelCount(channels);
		fmt.setCodec("audio/pcm");
		fmt.setByteOrder(QAudioFormat::LittleEndian);
		fmt.setSampleType(QAudioFormat::UnSignedInt);
		mux.lock();
		output = new QAudioOutput(fmt);
		io = output->start();	// 开始播放
		mux.unlock();

		if (io)
		{
			return true;
		}

		return false;
	}

	bool Close() override
	{
		mux.lock();
		if (io)
		{
			io->close();
			io = nullptr;
		}

		if (output)
		{
			output->stop();
			delete output;
			output = nullptr;
		}
		mux.unlock();
		return true;
	}

	bool Write(const unsigned char *data, int datasize) override
	{
		if (!data || datasize <= 0)
		{
			return false;
		}

		mux.lock();
		if (!output || !io)
		{
			mux.unlock();
			return false;
		}
		int size = io->write((const char *)data, datasize);
		mux.unlock();
		if (size != datasize)
		{
			return false;
		}

		return true;
	}

	int GetFree() override
	{
		mux.lock();
		if (!output)
		{
			mux.unlock();
			return 0;
		}
		int free = output->bytesFree();
		mux.unlock();

		return free;
	}

	long long GetNoPlayMs() override
	{
		mux.lock();
		if (!output)
		{
			mux.unlock();
			return 0;
		}
		long long pts = 0;
		// 还未播放的字节数
		double size = output->bufferSize() - output->bytesFree();
		// 1秒音频的字节大小
		double secSize = sampleRate * (sampleSize / 8)*channels;
		if (secSize <= 0.0)
		{
			pts = 0;
		}
		else
		{
			pts = (size / secSize) * 1000;
		}
		mux.unlock();

		return pts;
	}

	void SetPause(bool isPause) override
	{
		mux.lock();
		if (!output)
		{
			mux.unlock();
			return;
		}
		if (isPause)
		{
			output->suspend();
		}
		else
		{
			output->resume();
		}
		mux.unlock();
	}

	void Clear() override
	{
		mux.lock();
		if (io)
		{
			io->reset();
		}
		mux.unlock();
	}

public:
	QAudioOutput *output = nullptr;
	QIODevice *io = nullptr;
	std::mutex mux;
};

XAudioPlay::XAudioPlay()
{
}


XAudioPlay::~XAudioPlay()
{
}


XAudioPlay *XAudioPlay::Get()
{
	static CXAudioPlay play;

	return &play;
}
