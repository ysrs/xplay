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
    // ������ͣ
    virtual void SetPause(bool isPause) = 0;

    void set_sample_rate(int sample_rate)
    {
        sample_rate_ = sample_rate;
    }
    int sample_rate()
    {
        return sample_rate_;
    }

    void set_sample_size(int sample_size)
    {
        sample_size_ = sample_size;
    }
    int sample_size()
    {
        return sample_size_;
    }

    void set_channels(int channels)
    {
        channels_ = channels;
    }
    int channels()
    {
        return channels_;
    }

private:
    int sample_rate_ = 44100;
    int sample_size_ = 16;
    int channels_ = 2;
};

#endif // !XAUDIO_PLAY_H

