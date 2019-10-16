#pragma execution_character_set("utf-8")

#include "XPlay.h"
#include <QFileDialog>
#include <QMessageBox>

#include "XDemuxThread.h"


static XDemuxThread dt;

XPlay::XPlay(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	dt.Start();
	startTimer(40);
}


XPlay::~XPlay()
{
	dt.Close();
}


void XPlay::timerEvent(QTimerEvent *e)
{
	if (isSliderPress)
	{
		return;
	}

	long long total = dt.totalMs;
	if (total > 0)
	{
		double pos = (double)dt.pts / (double)total;
		int v = ui.playPos->maximum() * pos;
		ui.playPos->setValue(v);
	}
}

void XPlay::resizeEvent(QResizeEvent *e)
{
	ui.video->resize(this->size());
	
	ui.playPos->move(50, this->height() - 100);
	ui.playPos->resize(this->width() - 100, ui.playPos->height());

	ui.openFile->move(100, this->height() - 150);
	ui.isPlay->move(ui.openFile->x() + ui.openFile->width() + 10, ui.openFile->y());
}

void XPlay::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (isFullScreen())
	{
		this->showNormal();
	}
	else
	{
		this->showFullScreen();
	}
}

void XPlay::SetPause(bool isPause)
{
	if (isPause)
	{
        ui.isPlay->setText("播 放");
	}
	else
	{
        ui.isPlay->setText("暂 停");
	}
}

void XPlay::OpenFile()
{
	// 选择文件
    QString name = QFileDialog::getOpenFileName(this, "选择视频文件");
	//name = "rtmp://live.hkstv.hk.lxdns.com/live/hks1";
	if (name.isEmpty())
	{
		return;
	}

	this->setWindowTitle(name);
	if (!dt.Open(name.toLocal8Bit(), ui.video))
	{
		QMessageBox::information(nullptr, "error", "open file failed!");
	}

	SetPause(dt.isPause);
}

void XPlay::PlayOrPause()
{
	bool isPause = !dt.isPause;
	SetPause(isPause);
	dt.SetPause(isPause);
}

void XPlay::SliderPress()
{
	isSliderPress = true;
}

void XPlay::SliderRelease()
{
	isSliderPress = false;
	double pos = 0.0;
	pos = (double)ui.playPos->value() / (double)ui.playPos->maximum();
	dt.Seek(pos);
}

