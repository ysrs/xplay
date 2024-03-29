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

    // 输出参数和输入参数一致，除了采样格式，输出为S16，会释放para
    virtual bool Open(AVCodecParameters *para, bool isClearPara = false);
    // 关闭
    virtual void Close();
    // 返回重采样后大小，不管成功与否，都释放indata空间
    virtual int Resample(AVFrame *indata, unsigned char *d);

protected:
    std::mutex mux;
    SwrContext *actx = nullptr;

private:
    // AV_SAMPLE_FMT_S16
    int out_format_ = 1;
};

#endif // !XRESAMPLE_H

