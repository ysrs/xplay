#include "XDemuxThread.h"

#include <iostream>


#include "XDemux.h"
#include "XDecode.h"
#include "XVideoThread.h"
#include "XAudioThread.h"


using namespace std;

XDemuxThread::XDemuxThread()
{
}


XDemuxThread::~XDemuxThread()
{
	isExit = true;
	wait();
}


bool XDemuxThread::Open(const char *url, IVideoCall *call)
{
	if (nullptr == url || '\0' == url[0])
	{
		return false;
	}

	mux.lock();

	// �򿪽��װ
	bool re = demux->Open(url);
	if (!re)
	{
		cout << "demux->Open(url) failed!" << endl;
		return false;
	}
	// ����Ƶ�������ʹ����߳�
	if (!vt->Open(demux->CopyVPara(), call, demux->width, demux->height))
	{
		re = false;
		cout << "vt->Open() failed!" << endl;
	}
	// ����Ƶ�������ʹ����߳�
	if (!at->Open(demux->CopyAPara(), demux->sampleRate, demux->channels))
	{
		re = false;
		cout << "at->Open() failed!" << endl;
	}
	totalMs = demux->totalMs;
	mux.unlock();
	cout << "XDemuxThread::Open() re: " << re << endl;

	return re;
}

void XDemuxThread::Start()
{
	mux.lock();
	if (!demux)
	{
		demux = new XDemux();
	}
	if (!vt)
	{
		vt = new XVideoThread();
	}
	if (!at)
	{
		at = new XAudioThread();
	}

	// ������ǰ�߳�
	QThread::start();

	if (vt)
	{
		vt->start();
	}
	if(at)
	{
		at->start();
	}

	mux.unlock();
}

void XDemuxThread::Close()
{
	isExit = true;
	wait();

	if (vt)
	{
		vt->Close();

		mux.lock();
		delete vt;
		vt = nullptr;
		mux.unlock();
	}
	if (at)
	{
		at->Close();

		mux.lock();
		delete at;
		at = nullptr;
		mux.unlock();
	}
}

void XDemuxThread::Clear()
{
	mux.lock();
	if (demux)
	{
		demux->Clear();
	}
	if (vt)
	{
		vt->Clear();
	}
	if (at)
	{
		at->Clear();
	}
	mux.unlock();
}

void XDemuxThread::Seek(double pos)
{
	// ������
	Clear();

	mux.lock();
	bool status = this->isPause;
	mux.unlock();

	// ��ͣ
	SetPause(true);

	mux.lock();
	if (demux)
	{
		demux->Seek(pos);
	}

	// ʵ��Ҫ��ʾ��λ��
	long long seekPso = pos*demux->totalMs;
	while (!isExit)
	{
		AVPacket *pkt = demux->ReadVideo();
		if (!pkt)
		{
			break;
		}
		// ������뵽seekpts
		if (vt->RepaintPts(pkt, seekPso))
		{
			this->pts = seekPso;
			break;
		}
	}
	mux.unlock();

	// ���seekʱ�Ƿ���ͣ״̬����ô���ͻָ��ɲ���
	if (!status)
	{
		SetPause(false);
	}
}

void XDemuxThread::run()
{
	while (!isExit)
	{
		mux.lock();
		if (isPause)
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		if (!demux)
		{
			mux.unlock();
			msleep(5);
			continue;
		}
		// ����Ƶͬ��
		if (vt && at)
		{
			pts = at->pts;
			vt->synpts = at->pts;
		}

		AVPacket *pkt = demux->Read();
		if (!pkt)
		{
			mux.unlock();
			msleep(5);
			continue;
		}

		// �ж���������Ƶ
		if (demux->IsAudio(pkt))
		{
			if (at)
			{
				at->Push(pkt);
			}
		}
		else
		{
			// ��Ƶ
			if (vt)
			{
				vt->Push(pkt);
			}
		}

		mux.unlock();
		msleep(1);
	}
}

void XDemuxThread::SetPause(bool isPause)
{
	mux.lock();
	this->isPause = isPause;
	if (at)
	{
		at->SetPause(isPause);
	}
	if (vt)
	{
		vt->SetPause(isPause);
	}
	mux.unlock();
}

