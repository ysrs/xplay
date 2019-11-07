#ifndef XDECODE_H
#define XDECODE_H

#include <mutex>


struct AVCodecParameters;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

extern void XFreePacket(AVPacket **pkt);
extern void XFreeFrame(AVFrame **frame);

class XDecode
{
public:
	XDecode();
	virtual ~XDecode();
	// �򿪽����������ܳɹ���񣬶��ͷ�para�ռ�
	virtual bool Open(AVCodecParameters *para);
	virtual void Close();
	virtual void Clear();
	// ���͵������̣߳����ܳɹ���񣬶�����pkt�ռ䣨���������ý��ռ䣩
	virtual bool Send(AVPacket *pkt);
	// ��ȡ��������ݣ�һ��Send������Ҫ���Recv����ȡ�����е�����Send NULL��Recv���
	// ÿ�θ���һ�ݣ��ɵ������ͷţ�ʹ�ú���av_frame_free���ͷ�
	virtual AVFrame *Recv();

public:
	bool isAudio = false;
	// ��ǰ���뵽��pts
	long long pts = 0;

protected:
	AVCodecContext *codec = nullptr;
	std::mutex mux;
};

#endif // !XDECODE_H

