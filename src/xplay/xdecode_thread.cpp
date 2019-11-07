#include "xdecode_thread.h"
#include "xdecode.h"


XDecodeThread::XDecodeThread()
{
	// �򿪽�����
	if (!decode)
	{
		decode = new XDecode();
	}
}


XDecodeThread::~XDecodeThread()
{
	// �ȴ��߳��˳�
	isExit = true;
	wait();
}


void XDecodeThread::Close()
{
	Clear();
	
	// �ȴ��߳��˳�
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

	// ����
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

