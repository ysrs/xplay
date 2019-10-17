#pragma once

#include <QtWidgets/QWidget>
#include "ui_XPlay.h"

class XPlay : public QWidget
{
	Q_OBJECT

public:
	XPlay(QWidget *parent = Q_NULLPTR);
	~XPlay();

	// ��ʱ������������ʾ
	void timerEvent(QTimerEvent *e) override;
	// ���ڳߴ�仯
	void resizeEvent(QResizeEvent *e) override;
	// ˫��ȫ��
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


