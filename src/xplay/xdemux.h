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

public:
	// 媒体总时长
	int totalMs = 0;
	int width = 0;
	int height = 0;
	int sampleRate = 0;
	int channels = 0;

protected:
	std::mutex mux;
	// 解封装上下文
	AVFormatContext *ic = nullptr;
	// 音视频索引，读取时区分音视频
	int audioStream = 0;
	int videoStream = 1;
};

#endif // !XDEMUX_H

