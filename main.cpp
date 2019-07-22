#include <graphics.h>      // 引用图形库头文件
#include <conio.h>
#include "Graphics.h"
#include <stdio.h>
#include <time.h>
#pragma warning(disable:4996)
Matrix4 mvpMatrix;
Graphics *gp;
//vs fs的定义在Graph的头文件有说明
void vs(double const vbo[3],double* abo, Point4& Position)//顶点着色器
{

	double src[4];
	src[0] = vbo[0];
	src[1] = vbo[1];
	src[2] = vbo[2];
	src[3] = 1;

	Matrix::Mult(mvpMatrix.Value[0], src, 4, 1, 4, Position.value);//计算顶点坐标
}
void fs(double* ABO, COLORREF& FragColor)//片元着色器
{
	FragColor=gp->texture2D(ABO[0], ABO[1]);//因为ABO被设置成每个顶点两个属性，一个是纹理x坐标，一个是纹理y坐标
}
int main()
{
	gp=new Graphics(320, 240);	//创建一个画布
	gp->enable_CW = true;//启用顺时针逆时针三角形剔除
	gp->CW_CCW = false;//绘制逆时针三角形
	gp->VertexShader = vs;
	gp->FragmentShader = fs;
	//顶点
	double Vertrix[] = 
	{
		-0.5,0.5,0,-0.5,-0.5,0,0.5,-0.5,0,-0.5,0.5,0,0.5,-0.5,0,0.5,0.5,0,/*前面6个顶点坐标*/
		0.5,0.5,0,0.5,-0.5,0,0.5,-0.5,-1,0.5,0.5,0,0.5,-0.5,-1,0.5,0.5,-1,/*右面6个顶点坐标*/
		0.5,0.5,-1,0.5,-0.5,-1,-0.5,-0.5,-1,0.5,0.5,-1,-0.5,-0.5,-1,-0.5,0.5,-1,/*后面6个顶点坐标*/
		-0.5,0.5,-1,-0.5,-0.5,-1,-0.5,-0.5,0,-0.5,0.5,-1,-0.5,-0.5,0,-0.5,0.5,0,/*左面6个顶点坐标*/
		-0.5,0.5,-1,-0.5,0.5,0,0.5,0.5,0,-0.5,0.5,-1,0.5,0.5,0,0.5,0.5,-1,/*上面6个顶点坐标*/
		-0.5,-0.5,0,-0.5,-0.5,-1,0.5,-0.5,-1,-0.5,-0.5,0,0.5,-0.5,-1,0.5,-0.5,0/*下面6个顶点坐标*/
	};
	//各个顶点的纹理
	double textureCoordinate[] = 
	{
		0,1,0,0.75,0.25,0.75, 0,1,0.25,0.75,0.25,1,/*前面6个顶点纹理坐标*/
		0.25,1,0.25,0.75,0.5,0.75, 0.25,1,0.5,0.75,0.5,1,/*右面6个顶点纹理坐标*/
		0.5,1,0.5,0.75,0.75,0.75,  105,1,0.75,0.75,0.75,1,/*后面6个顶点纹理坐标*/
		0.75,1,0.75,0.75,1,0.75, 0.75,1,1,0.75,1,1,/*左面6个顶点纹理坐标*/
		0,0.75,0,0.5,0.25,0.5,0,0.75,0.25,0.5,0.25,0.75,/*上面6个顶点纹理坐标*/
		0.25,0.75,0.25,0.5,0.5,0.5, 0.25,0.75,0.5,0.5,0.5,0.75/*下面6个顶点纹理坐标*/
	};
	VBO v(Vertrix, 36);
	ABO a(textureCoordinate, 2, 36);
	gp->setABO(&a);
	gp->setVBO(&v);
	gp->LoadTexture("texture.png");

	/*坐标系是上面为Y正方向,右面为X正方向,屏幕向外为Z正方向*/
	Vector3 eyePosition(0, 0, 3); //相机原点
	Vector3 up(0, 1, 0); //相机上方向
	Vector3 destination(0, 0, -10000); //相机看向的点
	Matrix4 vMatrix = Matrix4::LookAt(eyePosition, up, destination);
	double l, r, b, t, n, f;
	double w = 5; //近平面的参照宽高
	double proportion;//宽高比
	if (gp->Width < gp->Height) 
	{ //宽度略小,以宽度为标准
		proportion = gp->Height / gp->Width;
		l = -w / 2;
		r = w / 2;
		b = -w * proportion / 2;
		t = w * proportion / 2;
		n = w;
		f = w * 50;
	}
	else 
	{
		proportion = gp->Width / gp->Height;
		l = -w * proportion / 2;
		r = w * proportion / 2;
		b = -w / 2;
		t = w / 2;
		n = w;
		f = w * 50;
	}
	Matrix4 pMatrix = Matrix4::PerspectiveProjection(l, r, b, t, n, f);
	Vector3 axis(1, 1, 1);//旋转轴
	Matrix4 vpMatrix;
	Matrix::Mult(pMatrix.Value[0], vMatrix.Value[0], 4, 4, 4, vpMatrix.Value[0]);
	char msg[256];
	for (int i = 0;;i++)
	{
		clock_t oldclock = clock();
		Matrix4 mMatrix = Matrix4::Rotate(axis, i);
		Matrix::Mult(vpMatrix.Value[0], mMatrix.Value[0], 4, 4, 4, mvpMatrix.Value[0]);
		gp->clearDepth();//清除深度缓冲区，将深度缓冲区值设置为一个非常小的值
		gp->SwapS();
		gp->clear();//清除屏幕
		gp->Draw();//绘制
		gp->flush();//等待同步
		gp->SwapE();//绘制到屏幕
		clock_t now = clock();
		if (16 - (now - oldclock) > 0)//如果绘制小于16毫秒，则休眠一段时间补齐16毫秒
		{
			Sleep(16 - (now - oldclock));
		}
		sprintf(msg, "%ld\n", now - oldclock);
		OutputDebugString(msg);//往调试器输出两帧绘制时间间隔
		oldclock = now;
	}
	delete gp;
}