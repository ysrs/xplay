#include "xaudio_thread.h"
#include "xdecode.h"
#include "xaudio_play.h"
#include "xresample.h"

#include <iostream>


using namespace std;
XAudioThread::XAudioThread()
{
	if (!res)
	{
		res = new XResample();
	}
	if (!ap)
	{
		ap = XAudioPlay::Get();
	}
}


XAudioThread::~XAudioThread()
{
	// �ȴ��߳��˳�
	isExit = true;
	wait();
}


void XAudioThread::run()
{
	unsigned char *pcm = new unsigned char[1024 * 1024 * 10];
	while (!isExit)
	{
		amux.lock();
		if (isPause)
		{
			amux.unlock();
			msleep(5);
			continue;
		}
		// û������
		//if (packs.empty() || !decode || !res || !ap)
		//{
		//	amux.unlock();
		//	msleep(1);
		//	continue;
		//}

		//AVPacket *pkt = packs.front();
		//packs.pop_front();

		AVPacket *pkt = Pop();

		bool re = decode->Send(pkt);
		if (!re)
		{
			amux.unlock();
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
			// ��ȥ������δ���ŵ�ʱ�䣬��λ��ms
			pts = decode->pts - ap->GetNoPlayMs();

			//cout << "audio pts = " << pts << endl;
			// �ز���
			int size = res->Resample(frame, pcm);

			// ������Ƶ
			while (!isExit)
			{
				if (size <= 0)
				{
					break;
				}
				// ����δ���꣬�ռ䲻��
				if (ap->GetFree() < size || isPause)
				{
					msleep(1);
					continue;
				}
				ap->Write(pcm, size);
				break;
			}
		}
		
		amux.unlock();
	}

	delete[] pcm;
}

bool XAudioThread::Open(AVCodecParameters *para, int sampleRate, int channels)
{
	if (!para)
	{
		return false;
	}

	Clear();

	amux.lock();
	pts = 0;
	bool re = true;
	if (!res->Open(para, false))
	{
		cout << "XAudioPlay open failed!" << endl;
		re = false;
	}

	ap->sampleRate = sampleRate;
	ap->channels = channels;
	if (!ap->Open())
	{
		cout << "XAudioPlay open failed!" << endl;
		re = false;
	}

	if (!decode->Open(para))
	{
		cout << "audio XDecode open failed!" << endl;
		re = false;
	}

	amux.unlock();
	cout << "XAudioThread::Open() re: " << re << endl;

	return re;
}

void XAudioThread::Close()
{
	XDecodeThread::Close();
	if (res)
	{
		res->Close();

		amux.lock();
		delete res;
		res = nullptr;
		amux.unlock();
	}

	if (ap)
	{
		ap->Close();

		amux.lock();
		ap = nullptr;
		amux.unlock();
	}
}

void XAudioThread::Clear()
{
	XDecodeThread::Clear();

	mux.lock();
	if (ap)
	{
		ap->Clear();
	}
	mux.unlock();
}

void XAudioThread::SetPause(bool isPause)
{
	//amux.lock();
	this->isPause = isPause;
	if (ap)
	{
		ap->SetPause(isPause);
	}
	//amux.unlock();
}

