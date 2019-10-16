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
	// 不管成功与否，都释放frame空间
	virtual void Repaint(AVFrame *frame) override;

protected:
	// 初始化gl
	void initializeGL() override;
	// 刷新显示
	void paintGL() override;
	// 窗口尺寸变化（指的是widget窗体变化）
	void resizeGL(int w, int h) override;

private:
	std::mutex mux;
	// shader程序
	QGLShaderProgram program;
	// shader中yuv变量地址
	GLuint unis[3] = { 0 };
	// OpenGL的texture地址
	GLuint texs[3] = { 0 };

	// 材质的内存空间
	unsigned char *datas[3] = { nullptr };

	int width = 240;
	int height = 128;
};
