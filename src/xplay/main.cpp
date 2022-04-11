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

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    XPlay w;
    w.show();

    return a.exec();
}
