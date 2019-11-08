#ifndef XDECODE_THREAD_H
#define XDECODE_THREAD_H

#include <QThread>
#include <mutex>
#include <list>


struct AVCodecParameters;
struct AVPacket;
class XDecode;
class XDecodeThread : public QThread
{
public:
    XDecodeThread();
    virtual ~XDecodeThread();
    // ����������Դ����ֹͣ�߳�
    virtual void Close();
    // �������
    virtual void Clear();
    virtual void Push(AVPacket *pkt);
    // ȡ��һ֡���ݣ�����ջ�����û�����ݣ�����nullptr
    virtual AVPacket *Pop();

public:
    // ������
    int maxList = 100;
    bool isExit = false;

protected:
    std::list<AVPacket *> packs_;
    std::mutex mutex_;
    XDecode *decode_ = nullptr;
};

#endif // !XDECODE_THREAD_H

