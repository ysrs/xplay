#include "XVideoWidget.h"
#include <QDebug>
#include <QTimer>

extern "C"
{
#include <libavutil/frame.h>
}

FILE *fp = nullptr;


#define A_VER	3
#define T_VER	4

// �Զ���˫����
#define GET_STR(x) #x

// ƬԪshader
const char *tString = GET_STR(
	varying vec2 textureOut;
uniform sampler2D tex_y;
uniform sampler2D tex_u;
uniform sampler2D tex_v;
void main(void)
{
	vec3 yuv;
	vec3 rgb;
	yuv.x = texture2D(tex_y, textureOut).r;
	yuv.y = texture2D(tex_u, textureOut).r - 0.5;
	yuv.z = texture2D(tex_v, textureOut).r - 0.5;
	rgb = mat3(1.0, 1.0, 1.0,
		0.0, -0.39465, 2.03211,
		1.13983, -0.58060, 0.0) * yuv;
	gl_FragColor = vec4(rgb, 1.0);
}
);

// ����shader
const char *vString = GET_STR(
	attribute vec4 vertexIn;
attribute vec2 textureIn;
varying vec2 textureOut;
void main(void)
{
	gl_Position = vertexIn;
	textureOut = textureIn;
}
);

// ׼��yuv����
// ffmpeg -i v1080.mp4 -t 10 -s 240x128 -pix_fmt yuv420p out240x128.yuv
XVideoWidget::XVideoWidget(QWidget *parent)
	: QOpenGLWidget(parent)
{
}

XVideoWidget::~XVideoWidget()
{
}

void XVideoWidget::Init(int width, int height)
{
	mux.lock();
	this->width = width;
	this->height = height;

	delete datas[0];
	delete datas[1];
	delete datas[2];

	// ��������ڴ�ռ�
	datas[0] = new unsigned char[width*height];		// Y
	datas[1] = new unsigned char[width*height / 4];	// U
	datas[2] = new unsigned char[width*height / 4];	// V


	if (texs[0])
	{
		glDeleteTextures(3, texs);
	}
	// ��������
	glGenTextures(3, texs);

	// Y
	glBindTexture(GL_TEXTURE_2D, texs[0]);
	// �Ŵ�������Բ�ֵ��    GL_NEAREST��Ч�ʸߣ������������أ�
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// ���������Կ��ռ�
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	// U
	glBindTexture(GL_TEXTURE_2D, texs[1]);
	// �Ŵ�������Բ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// ���������Կ��ռ�
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

	// V
	glBindTexture(GL_TEXTURE_2D, texs[2]);
	// �Ŵ�������Բ�ֵ
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// ���������Կ��ռ�
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	mux.unlock();
}

void XVideoWidget::Repaint(AVFrame *frame)
{
	if (!frame)
	{
		return;
	}

	mux.lock();
	// �ݴ���֤�ߴ���ȷ
	if (!datas[0] || width*height == 0 || frame->width != width || frame->height != height)
	{
		av_frame_free(&frame);
		mux.unlock();
		return;
	}
	// �������
	if (width == frame->linesize[0])
	{
		memcpy(datas[0], frame->data[0], width*height);
		memcpy(datas[1], frame->data[1], width*height / 4);
		memcpy(datas[2], frame->data[2], width*height / 4);
	}
	else
	{
		// �ж�������
		// Y
		for (int i = 0; i < height; ++i)
		{
			memcpy(datas[0] + width*i, frame->data[0] + frame->linesize[0] * i, width);
		}
		// U
		for (int i = 0; i < height / 2; ++i)
		{
			memcpy(datas[1] + width / 2 * i, frame->data[1] + frame->linesize[1] * i, width);
		}
		// V
		for (int i = 0; i < height / 2; ++i)
		{
			memcpy(datas[2] + width / 2 * i, frame->data[2] + frame->linesize[2] * i, width);
		}
	}

	
	mux.unlock();

	av_frame_free(&frame);

	update();
}

void XVideoWidget::initializeGL()
{
	mux.lock();
	qDebug() << "initializeGL";
	// ��ʼ��opengl OpenGLFunctions�̳к���
	initializeOpenGLFunctions();

	// program����shader�ű����������ƬԪ���ű�
	// ƬԪ�����أ�
	qDebug() << program.addShaderFromSourceCode(QGLShader::Fragment, tString);
	// ����shader
	qDebug() << program.addShaderFromSourceCode(QGLShader::Vertex, vString);

	// ���ö�������ı���
	program.bindAttributeLocation("vertexIn", A_VER);
	// ���ò�������
	program.bindAttributeLocation("textureIn", T_VER);
	// ����shader
	qDebug() << "program.link(): " << program.link();
	qDebug() << "program.bind(): " << program.bind();

	// ���ݶ���Ͳ�������
	// ����
	static const GLfloat ver[] = {
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f, 1.0f,
		1.0f, 1.0f
	};
	// ��������
	static const GLfloat tex[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f
	};

	// ����
	glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
	glEnableVertexAttribArray(A_VER);

	// ����
	glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
	glEnableVertexAttribArray(T_VER);


	// ��shader��ȡ����
	unis[0] = program.uniformLocation("tex_y");
	unis[1] = program.uniformLocation("tex_u");
	unis[2] = program.uniformLocation("tex_v");

	mux.unlock();

	

	//fp = fopen("out240x128.yuv", "rb");
	//if (!fp)
	//{
	//	qDebug() << "out240x128.yuv file open failed!";
	//}

	//// ������ʱ��
	//QTimer *ti = new QTimer(this);
	//connect(ti, SIGNAL(timeout()), this, SLOT(update()));
	//ti->start(40);
}

void XVideoWidget::paintGL()
{
	//if (feof(fp))
	//{
	//	fseek(fp, 0, SEEK_SET);
	//}

	//fread(datas[0], 1, width*height, fp);
	//fread(datas[1], 1, width*height / 4, fp);
	//fread(datas[2], 1, width*height / 4, fp);

	mux.lock();

	glActiveTexture(GL_TEXTURE0);
	// 0��󶨵�Y����
	glBindTexture(GL_TEXTURE_2D, texs[0]);
	// �޸Ĳ������ݣ������ڴ����ݣ�
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, datas[0]);
	// ��shader��uni��������
	glUniform1i(unis[0], 0);

	glActiveTexture(GL_TEXTURE0 + 1);
	// 1��󶨵�Y����
	glBindTexture(GL_TEXTURE_2D, texs[1]);
	// �޸Ĳ������ݣ������ڴ����ݣ�
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[1]);
	// ��shader��uni��������
	glUniform1i(unis[1], 1);

	glActiveTexture(GL_TEXTURE0 + 2);
	// 2��󶨵�Y����
	glBindTexture(GL_TEXTURE_2D, texs[2]);
	// �޸Ĳ������ݣ������ڴ����ݣ�
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, datas[2]);
	// ��shader��uni��������
	glUniform1i(unis[2], 2);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	qDebug() << "paintGL";

	mux.unlock();
}

void XVideoWidget::resizeGL(int w, int h)
{
	mux.lock();
	qDebug() << "resizeGL: " << w << " x " << h;
	mux.unlock();
}


