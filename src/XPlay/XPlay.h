#pragma once

#include <QtWidgets/QWidget>
#include "ui_XPlay.h"

class XPlay : public QWidget
{
	Q_OBJECT

public:
	XPlay(QWidget *parent = Q_NULLPTR);
	~XPlay();

	// 定时器，滑动条显示
	void timerEvent(QTimerEvent *e) override;
	// 窗口尺寸变化
	void resizeEvent(QResizeEvent *e) override;
	// 双击全屏
	void mouseDoubleClickEvent(QMouseEvent *e) override;
	void SetPause(bool isPause);

private:
	bool isSliderPress = false;
	
public slots:
	void OpenFile();
	void PlayOrPause();
	void SliderPress();
	void SliderRelease();

private:
	Ui::XPlayClass ui;
};


