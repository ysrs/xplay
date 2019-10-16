#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>

#include "IVideoCall.h"


struct AVFrame;

class XVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{
	Q_OBJECT

public:
	XVideoWidget(QWidget *parent);
	~XVideoWidget();

	void Init(int width, int height) override;
	// ���ܳɹ���񣬶��ͷ�frame�ռ�
	virtual void Repaint(AVFrame *frame) override;

protected:
	// ��ʼ��gl
	void initializeGL() override;
	// ˢ����ʾ
	void paintGL() override;
	// ���ڳߴ�仯��ָ����widget����仯��
	void resizeGL(int w, int h) override;

private:
	std::mutex mux;
	// shader����
	QGLShaderProgram program;
	// shader��yuv������ַ
	GLuint unis[3] = { 0 };
	// OpenGL��texture��ַ
	GLuint texs[3] = { 0 };

	// ���ʵ��ڴ�ռ�
	unsigned char *datas[3] = { nullptr };

	int width = 240;
	int height = 128;
};
