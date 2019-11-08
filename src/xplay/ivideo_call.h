#ifndef IVIDEO_CALL_H
#define IVIDEO_CALL_H


struct AVFrame;
class IVideoCall
{
public:
    virtual void Init(int width, int height) = 0;
    virtual void Repaint(AVFrame *frame) = 0;
};

#endif // !IVIDEO_CALL_H

