#include <graphics.h>      // 引用图形库头文件
#include <conio.h>
#include "Graphics.h"
#include <stdio.h>
int main()
{
	Graphics gp(320, 240);	//创建一个画布
	double Vertrix[] = {
		-0.5,0.5,0,-0.5,-0.5,0,0.5,-0.5,0,/*前面*/-0.5,0.5,0,0.5,-0.5,0,0.5,0.5,0,
		0.5,0.5,0,0.5,-0.5,0,0.5,-0.5,-1,/*右面*/0.5,0.5,0,0.5,-0.5,-1,0.5,0.5,-1,
		0.5,0.5,-1,0.5,-0.5,-1,-0.5,-0.5,-1,/*后面*/0.5,0.5,-1,-0.5,-0.5,-1,-0.5,0.5,-1,
		-0.5,0.5,-1,-0.5,-0.5,-1,-0.5,-0.5,0,/*左面*/-0.5,0.5,-1,-0.5,-0.5,0,-0.5,0.5,0,
		-0.5,0.5,-1,-0.5,0.5,0,0.5,0.5,0,/*上面*/-0.5,0.5,-1,0.5,0.5,0,0.5,0.5,-1,
		-0.5,-0.5,0,-0.5,-0.5,-1,0.5,-0.5,-1,/*下面*/-0.5,-0.5,0,0.5,-0.5,-1,0.5,-0.5,0
	};
	//纹素坐标Y向上增加
	double textureCoordinate[] = {
		0,1,0,0.75,0.25,0.75, 0,1,0.25,0.75,0.25,1,/*前面*/
		0,1,0,0,1,0, 0,1,1,0,1,1,/*右面*/
		0,1,0,0,1,0, 0,1,1,0,1,1,/*后面*/
		0,1,0,0,1,0, 0,1,1,0,1,1,/*左面*/
		0,1,0,0,1,0, 0,1,1,0,1,1,/*左面*/
		0,1,0,0,1,0, 0,1,1,0,1,1/*下面*/
	};
	VBO v(Vertrix,36);
	ABOd a(textureCoordinate,2,36);
	gp.setTextureCoordinate(&a);
	gp.setVBO(&v);
	gp.LoadTexture("texture.png");

	/*坐标系是上面为Y正方向，右面为X正方向，屏幕向外为Z正方向*/
	Vector3 eyePosition(0, 0, 10); //相机原点
	Vector3 up(0, 1, 0); //相机上方向
	Vector3 destination(0, 0, -10000); //相机看向的点
	Matrix4 vMatrix = Matrix4::LookAt(eyePosition, up, destination);
	double l, r, b, t, n, f;
	double w = 5; //近平面的参照宽高
	double proportion;//宽高比
	if (gp.Width < gp.Height) { //宽度略小，以宽度为标准
		proportion = gp.Height / gp.Width;
		l = -w / 2;
		r = w / 2;
		b = -w * proportion / 2;
		t = w * proportion / 2;
		n = w;
		f = w * 50;
	}
	else {
		proportion = gp.Width / gp.Height;
		l = -w * proportion / 2;
		r = w * proportion / 2;
		b = -w / 2;
		t = w / 2;
		n = w;
		f = w * 50;
	}
	Matrix4 pMatrix = Matrix4::PerspectiveProjection(l, r, b, t, n, f);
	Vector3 axis(0, 1, 1);//旋转轴
	Matrix4 vpMatrix;
	Matrix::Mult(pMatrix.Value[0],vMatrix.Value[0],4,4,4,vpMatrix.Value[0]);
	Matrix4 mvpMatrix;
	for (int i=0;;i++)
	{
		Matrix4 mMatrix = Matrix4::Rotate(axis, i);
		Matrix::Mult(vpMatrix.Value[0], mMatrix.Value[0], 4, 4, 4, mvpMatrix.Value[0]);
		gp.MVPMatrix = mvpMatrix;
		gp.clearDepth();
		gp.clear();//清除屏幕
		gp.Draw();//绘制，这里会闪烁
		gp.flush();
		Sleep(50);
	}
	_getch();// 按任意键继续
}