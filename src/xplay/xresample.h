#ifndef XRESAMPLE_H
#define XRESAMPLE_H

#include <mutex>


struct AVCodecParameters;
struct SwrContext;
struct AVFrame;
class XResample
{
public:
    XResample();
    ~XResample();

    // ����������������һ�£����˲�����ʽ�����ΪS16�����ͷ�para
    virtual bool Open(AVCodecParameters *para, bool isClearPara = false);
    // �ر�
    virtual void Close();
    // �����ز������С�����ܳɹ���񣬶��ͷ�indata�ռ�
    virtual int Resample(AVFrame *indata, unsigned char *d);

protected:
    std::mutex mux;
    SwrContext *actx = nullptr;

private:
    // AV_SAMPLE_FMT_S16
    int out_format_ = 1;
};

#endif // !XRESAMPLE_H

