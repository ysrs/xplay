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

    // 打开媒体文件或者流媒体 rtmp http rstp
    virtual bool Open(const char *url);
    // 空间需要调用者释放，释放AVPacket对象空间和数据空间，使用av_packet_free函数释放
    virtual AVPacket *Read();
    // 只读视频，音频丢弃，空间释放
    virtual AVPacket *ReadVideo();
    // 获取视频参数，返回的空间需要清理，使用av_codec_parameters_free函数
    virtual AVCodecParameters *CopyVPara();
    // 获取音频参数，返回的空间需要清理，使用av_codec_parameters_free函数
    virtual AVCodecParameters *CopyAPara();
    // seek 位置，范围在0.0~1.0之间
    virtual bool Seek(double pos);
    // 清空读取缓存
    virtual void Clear();
    // 关闭
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
    // 解封装上下文
    AVFormatContext *ic_ = nullptr;
    // 音视频索引，读取时区分音视频
    int audio_stream_ = 0;
    int video_stream_ = 1;

private:
    // 媒体总时长
    int total_ms_ = 0;
    int width_ = 0;
    int height_ = 0;
    int sample_rate_ = 0;
    int channels_ = 0;
};

#endif // !XDEMUX_H

