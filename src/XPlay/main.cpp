#include "XPlay.h"
#include <QtWidgets/QApplication>
#include <QThread>
#include <iostream>

#include "XDemux.h"
#include "XDecode.h"
#include "XResample.h"
#include "XAudioPlay.h"
#include "XAudioThread.h"
#include "XVideoThread.h"

#include "XDemuxThread.h"



// 1920x1080.mp4
using namespace std;

class TestThread : public QThread
{
public:
	void Init()
	{
		// "rtmp://live.hkstv.hk.lxdns.com/live/hks"
		//char *url = "http://ivi.bupt.edu.cn/hls/cctv3hd.m3u8";
        const char *url = "1920x1080.mp4";
		cout << "demux.Open: " << demux.Open(url) << endl;
		demux.Read();
		demux.Clear();
		demux.Close();

		cout << "demux.Open: " << demux.Open(url) << endl;
		cout << "demux.CopyVPara: " << demux.CopyVPara() << endl;
		cout << "demux.CopyAPara: " << demux.CopyAPara() << endl;
		cout << "demux.Seek: " << demux.Seek(0.05) << endl;

		
		//cout << "vdecode.Open(): " << vdecode.Open(demux.CopyVPara()) << endl;
		//vdecode.Clear();
		//vdecode.Close();
		//cout << "adecode.Open(): " << adecode.Open(demux.CopyAPara()) << endl;
		//cout << "resample.Ope(): " << resample.Open(demux.CopyAPara()) << endl;
		//XAudioPlay::Get()->channels = demux.channels;
		//XAudioPlay::Get()->sampleRate = demux.sampleRate;
		//cout << "XAudioPlay::Get()->Open(): " << XAudioPlay::Get()->Open() << endl;

		cout << "at.Open(): " << at.Open(demux.CopyAPara(), demux.sampleRate, demux.channels) << endl;
		cout << "at.Open(): " << vt.Open(demux.CopyVPara(), video, demux.width, demux.height) << endl;
		at.start();
		vt.start();
	}

	void run() override
	{
		unsigned char *pcm = new unsigned char[1024 * 1024];
		for (;;)
		{
			AVPacket *pkt = demux.Read();
			if (demux.IsAudio(pkt))
			{
				at.Push(pkt);

				//adecode.Send(pkt);
				//AVFrame *frame = adecode.Recv();
				//int len = resample.Resample(frame, pcm);
				//cout << "Resample: " << len << " ";
				//while (len > 0)
				//{
				//	if (XAudioPlay::Get()->GetFree() >= len)
				//	{
				//		XAudioPlay::Get()->Write(pcm, len);
				//		break;
				//	}
				//	msleep(1);
				//}

				//cout << "Audio: " << frame << endl;
			}
			else
			{
				vt.Push(pkt);

				//vdecode.Send(pkt);
				//AVFrame *frame = vdecode.Recv();
				//video->Repaint(frame);
				//msleep(40);
				//cout << "Video: " << frame << endl;
			}

			if (!pkt)
			{
				break;
			}

			//av_packet_free(&pkt);
		}
	}

	// ²âÊÔXDemux
	XDemux demux;
	// ½âÂë²âÊÔ
	//XDecode vdecode;
	//XDecode adecode;
	//XResample resample;
	XVideoWidget *video = nullptr;
	XAudioThread at;
	XVideoThread vt;
};

int main(int argc, char *argv[])
{
	//TestThread tt;

	QApplication a(argc, argv);
	XPlay w;
	w.show();

	// ³õÊ¼»¯gl´°¿Ú
	//w.ui.video->Init(tt.demux.width, tt.demux.height);
	//tt.video = w.ui.video;
	//tt.Init();
	//tt.start();


	//char *url = "http://ivi.bupt.edu.cn/hls/cctv3hd.m3u8";
	//char *url = "1920x1080.mp4";
	//XDemuxThread dt;
	//dt.Open(url, w.ui.video);
	//dt.Start();

	return a.exec();
}
