#include "xvideo_thread.h"
#include "XDecode.h"

#include <iostream>


using namespace std;

XVideoThread::XVideoThread()
{
}


XVideoThread::~XVideoThread()
{
}


void XVideoThread::run()
{
	while (!isExit)
	{
		vmux.lock();
		if (isPause)
		{
			vmux.unlock();
			msleep(5);
			continue;
		}

		// ����Ƶͬ��
		if ((synpts > 0) && (synpts < decode->pts))
		{
			vmux.unlock();
			msleep(1);
			continue;
		}

		// û������
		//if (packs.empty() || !decode)
		//{
		//	vmux.unlock();
		//	msleep(1);
		//	continue;
		//}
		//
		//AVPacket *pkt = packs.front();
		//packs.pop_front();

		AVPacket *pkt = Pop();


		bool re = decode->Send(pkt);
		if (!re)
		{
			vmux.unlock();
			msleep(1);
			continue;;
		}
		// һ��Send�����Recv
		while (!isExit)
		{
			AVFrame *frame = decode->Recv();
			if (!frame)
			{
				break;
			}
			// ��ʾ��Ƶ
			if (call)
			{
				call->Repaint(frame);
			}
		}

		vmux.unlock();
	}
}

bool XVideoThread::Open(AVCodecParameters *para, IVideoCall *call, int width, int height)
{
	if (!para)
	{
		return false;
	}

	Clear();

	vmux.lock();
	synpts = 0;
	// ��ʼ����ʾ����
	this->call = call;
	if (call)
	{
		call->Init(width, height);
	}
	vmux.unlock();

	bool re = true;
	if (!decode->Open(para))
	{
		cout << "video XDecode open failed!" << endl;
		re = false;
	}
	
	cout << "XVideoThread::Open() re: " << re << endl;

	return re;
}

bool XVideoThread::RepaintPts(AVPacket *pkt, long long seekpts)
{
	vmux.lock();

	bool re = decode->Send(pkt);
	if (!re)
	{
		// ��ʾ��������
		vmux.unlock();
		return true;
	}
	AVFrame *frame = decode->Recv();
	if (!frame)
	{
		vmux.unlock();
		return false;
	}
	// ����λ��
	if (decode->pts >= seekpts)
	{
		if (call)
		{
			call->Repaint(frame);
		}
		vmux.unlock();
		return true;
	}
	XFreeFrame(&frame);
	vmux.unlock();

	return false;
}

void XVideoThread::SetPause(bool isPause)
{
	vmux.lock();
	this->isPause = isPause;
	vmux.unlock();
}

