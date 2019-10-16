#pragma once

#include <QObject>
#include <QMouseEvent>
#include <QSlider>

class XSlider : public QSlider
{
	Q_OBJECT

public:
	XSlider(QWidget *parent = nullptr);
	~XSlider();

	void mousePressEvent(QMouseEvent *e) override;
};
