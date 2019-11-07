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

public:
	// ý����ʱ��
	int totalMs = 0;
	int width = 0;
	int height = 0;
	int sampleRate = 0;
	int channels = 0;

protected:
	std::mutex mux;
	// ���װ������
	AVFormatContext *ic = nullptr;
	// ����Ƶ��������ȡʱ��������Ƶ
	int audioStream = 0;
	int videoStream = 1;
};

#endif // !XDEMUX_H

