#include "XResample.h"
#include <iostream>

extern "C"
{
#include <libswresample/swresample.h>
#include <libavcodec/avcodec.h>
}

#ifdef _WIN32
#pragma comment(lib, "swresample.lib")
#endif

using namespace std;
XResample::XResample()
{
}


XResample::~XResample()
{
}

bool XResample::Open(AVCodecParameters *para, bool isClearPara)
{
	if (!para)
	{
		return false;
	}
	mux.lock();
	// ��Ƶ�ز��� �����ĳ�ʼ��
	// ���actxΪnullptr�����Զ�����ռ�
	actx = swr_alloc_set_opts(
		actx,
		av_get_default_channel_layout(2),				// �����ʽ
		(AVSampleFormat)outFormat,						// ���������ʽ 1 AV_SAMPLE_FMT_S16
		para->sample_rate,								// �����ʣ�1���ӵ���Ƶ��������
		av_get_default_channel_layout(para->channels),	// �����ʽ
		(AVSampleFormat)para->format,
		para->sample_rate,
		0,
		nullptr
	);
	if (isClearPara)
	{
		avcodec_parameters_free(&para);
	}
	int re = swr_init(actx);
	mux.unlock();
	if (re != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(re, buf, sizeof(buf) - 1);
		cout << "audio swr_init failed! :" << buf << endl;
		return false;
	}

	return true;
}

void XResample::Close()
{
	mux.lock();

	if (actx)
	{
		swr_free(&actx);
	}

	mux.unlock();
}

int XResample::Resample(AVFrame *indata, unsigned char *d)
{
	if (!indata)
	{
		return 0;
	}

	if (!d)
	{
		av_frame_free(&indata);
		return 0;
	}

	uint8_t *data[2] = { nullptr };
	data[0] = d;
	int re = swr_convert(
		actx,
		data,
		indata->nb_samples,					// ���
		(const uint8_t **)indata->data,		// ����
		indata->nb_samples
	);
	int outSize = re * indata->channels * av_get_bytes_per_sample((AVSampleFormat)outFormat);
	// ��ߵ����ڴ�й©
	av_frame_free(&indata);
	if (re <= 0)
	{
		return re;
	}

	return outSize;
}
