#ifndef XDEMUX_H
#define XDEMUX_H

#include <mutex>


struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;
class XDemux
{
public:
    XDemux();
    virtual ~XDemux();

    // ��ý���ļ�������ý�� rtmp http rstp
    virtual bool Open(const char *url);
    // �ռ���Ҫ�������ͷţ��ͷ�AVPacket����ռ�����ݿռ䣬ʹ��av_packet_free�����ͷ�
    virtual AVPacket *Read();
    // ֻ����Ƶ����Ƶ�������ռ��ͷ�
    virtual AVPacket *ReadVideo();
    // ��ȡ��Ƶ���������صĿռ���Ҫ����ʹ��av_codec_parameters_free����
    virtual AVCodecParameters *CopyVPara();
    // ��ȡ��Ƶ���������صĿռ���Ҫ����ʹ��av_codec_parameters_free����
    virtual AVCodecParameters *CopyAPara();
    // seek λ�ã���Χ��0.0~1.0֮��
    virtual bool Seek(double pos);
    // ��ն�ȡ����
    virtual void Clear();
    // �ر�
    virtual void Close();

    virtual bool IsAudio(AVPacket *pkt);


    void set_total_ms(int total_ms)
    {
        total_ms_ = total_ms;
    }
    int total_ms()
    {
        return total_ms_;
    }

    void set_width(int width)
    {
        width_ = width;
    }
    int width()
    {
        return width_;
    }

    void set_height(int height)
    {
        height_ = height;
    }
    int height()
    {
        return height_;
    }

    void set_sample_rate(int sample_rate)
    {
        sample_rate_ = sample_rate;
    }
    int sample_rate()
    {
        return sample_rate_;
    }

    void set_channels(int channels)
    {
        channels_ = channels;
    }
    int channels()
    {
        return channels_;
    }

protected:
    std::mutex mutex_;
    // ���װ������
    AVFormatContext *ic_ = nullptr;
    // ����Ƶ��������ȡʱ��������Ƶ
    int audio_stream_ = 0;
    int video_stream_ = 1;

private:
    // ý����ʱ��
    int total_ms_ = 0;
    int width_ = 0;
    int height_ = 0;
    int sample_rate_ = 0;
    int channels_ = 0;
};

#endif // !XDEMUX_H

