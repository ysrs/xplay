#include "xdecode_thread.h"
#include "xdecode.h"


XDecodeThread::XDecodeThread()
{
	// 打开解码器
	if (!decode)
	{
		decode = new XDecode();
	}
}


XDecodeThread::~XDecodeThread()
{
	// 等待线程退出
	isExit = true;
	wait();
}


void XDecodeThread::Close()
{
	Clear();
	
	// 等待线程退出
	isExit = true;
	wait();

	mux.lock();
	decode->Close();
	delete decode;
	decode = nullptr;
	mux.unlock();
}

void XDecodeThread::Clear()
{
	mux.lock();
	decode->Clear();
	while (!packs.empty())
	{
		AVPacket *pkt = packs.front();
		XFreePacket(&pkt);
		packs.pop_front();
	}
	mux.unlock();
}

void XDecodeThread::Push(AVPacket *pkt)
{
	if (!pkt)
	{
		return;
	}

	// 阻塞
	while (!isExit)
	{
		mux.lock();
		if (packs.size() < maxList)
		{
			packs.push_back(pkt);
			mux.unlock();
			break;
		}
		mux.unlock();
		msleep(1);
	}
}

AVPacket *XDecodeThread::Pop()
{
	mux.lock();

	if (packs.empty())
	{
		mux.unlock();
		return nullptr;
	}
	AVPacket *pkt = packs.front();
	packs.pop_front();
	mux.unlock();

	return pkt;
}

