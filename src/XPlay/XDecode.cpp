#include "XDecode.h"
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
}


using namespace std;

void XFreePacket(AVPacket **pkt)
{
	if (!pkt || !(*pkt))
	{
		return;
	}
	av_packet_free(pkt);
}

void XFreeFrame(AVFrame **frame)
{
	if (!frame || !(*frame))
	{
		return;
	}
	av_frame_free(frame);
}

XDecode::XDecode()
{
}


XDecode::~XDecode()
{
}


bool XDecode::Open(AVCodecParameters *para)
{
	if (!para)
	{
		return false;
	}
	Close();

	// 解码器打开
	// 找到解码器
	AVCodec *vcodec = avcodec_find_decoder(para->codec_id);
	if (!vcodec)
	{
		avcodec_parameters_free(&para);
		cout << "can't find the codec id " << para->codec_id << endl;
		return false;
	}
	cout << "find the AVCodec " << para->codec_id << endl;

	// 创建解码器上下文
	mux.lock();
	codec = avcodec_alloc_context3(vcodec);
	/// 配置解码器上下文参数
	avcodec_parameters_to_context(codec, para);
	avcodec_parameters_free(&para);
	// 8线程解码
	codec->thread_count = 8;
	// 打开解码器上下文
	int re = avcodec_open2(codec, nullptr, nullptr);
	if (re != 0)
	{
		avcodec_free_context(&codec);
		mux.unlock();
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "avcodec_open2 failed! :" << buf << endl;
		return false;
	}
	mux.unlock();
	cout << "video avcodec_open2 success!" << endl;

	return true;
}

void XDecode::Close()
{
	mux.lock();
	if (codec)
	{
		avcodec_close(codec);
		avcodec_free_context(&codec);
	}
	pts = 0;
	mux.unlock();
}

void XDecode::Clear()
{
	mux.lock();
	// 清理解码缓冲
	if (codec)
	{
		avcodec_flush_buffers(codec);
	}
	mux.unlock();
}

bool XDecode::Send(AVPacket *pkt)
{
	// 容错处理
	if (!pkt || pkt->size<=0 || !pkt->data)
	{
		return false;
	}
	mux.lock();
	if (!codec)
	{
		mux.unlock();
		return false;
	}
	// 每次调用后都有几k到几百k的内存泄漏，不知道怎么回事？？？？？？？？？？？？？？？？？
	// 找到问题了，在XResample::Resample函数中泄漏的
	int re = avcodec_send_packet(codec, pkt);
	mux.unlock();
	av_packet_free(&pkt);
	if (re != 0)
	{
		return false;
	}

	return true;
}

AVFrame *XDecode::Recv()
{
	mux.lock();
	if (!codec)
	{
		mux.unlock();
		return nullptr;
	}
	AVFrame *frame = av_frame_alloc();
	int re = avcodec_receive_frame(codec, frame);
	mux.unlock();

	if (re != 0)
	{
		av_frame_free(&frame);
		return nullptr;
	}
	//cout << "[" << frame->linesize[0] << "] " << flush;
	pts = frame->pts;
	return frame;
}


