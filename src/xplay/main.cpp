#include "XPlay.h"
#include <QtWidgets/QApplication>
#include <QThread>
#include <iostream>

#include "xdemux.h"
#include "xdecode.h"
#include "xresample.h"
#include "xaudio_play.h"
#include "xaudio_thread.h"
#include "xvideo_thread.h"
#include "xdemux_thread.h"


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
        cout << "at.Open(): " << at.Open(demux.CopyAPara(), demux.sample_rate(), demux.channels()) << endl;
        cout << "at.Open(): " << vt.Open(demux.CopyVPara(), video, demux.width(), demux.height()) << endl;
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
            }
            else
            {
                vt.Push(pkt);
            }

            if (!pkt)
            {
                break;
            }
        }
    }

    XDemux demux;
    XVideoWidget *video = nullptr;
    XAudioThread at;
    XVideoThread vt;
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XPlay w;
    w.show();

    return a.exec();
}
