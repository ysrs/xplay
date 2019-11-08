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
    // ԭ���¼�����
    //QSlider::mousePressEvent(e);
    QSlider::sliderReleased();
}
