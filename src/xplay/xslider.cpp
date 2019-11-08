#include "XSlider.h"

XSlider::XSlider(QWidget *parent)
    : QSlider(parent)
{
}

XSlider::~XSlider()
{
}


void XSlider::mousePressEvent(QMouseEvent *e)
{
    double pos = (double)e->pos().x() / (double)width();
    int p = pos * this->maximum();
    setValue(p);
    // 原有事件处理
    //QSlider::mousePressEvent(e);
    QSlider::sliderReleased();
}
