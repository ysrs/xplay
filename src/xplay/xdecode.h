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
	// 打开解码器，不管成功与否，都释放para空间
	virtual bool Open(AVCodecParameters *para);
	virtual void Close();
	virtual void Clear();
	// 发送到解码线程，不管成功与否，都清理pkt空间（包括对象和媒体空间）
	virtual bool Send(AVPacket *pkt);
	// 获取解码后数据，一次Send可能需要多次Recv，获取缓冲中的数据Send NULL再Recv多次
	// 每次复制一份，由调用者释放，使用函数av_frame_free来释放
	virtual AVFrame *Recv();

public:
	bool isAudio = false;
	// 当前解码到的pts
	long long pts = 0;

protected:
	AVCodecContext *codec = nullptr;
	std::mutex mux;
};

#endif // !XDECODE_H

