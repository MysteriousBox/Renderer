#ifndef _Graphics
#define _Graphics
#include <graphics.h>      // 引用图形库头文件
#include "Point2.h"
#include "Point4.h"
#include "EdgeTableItem.h"
#include "Matrix4.h"
class VBO//vertex buffer object
{
public:
	double *Buffer;//数据必须按照x1,y1,z1,x2,y2,z2.......这样的顺序存放
	unsigned int Count = 0;
	VBO(double* buffer, unsigned int count);//顶点个数
	~VBO();
};
class ABOd//Attribute buffer object(double),暂时只能用于设置纹理坐标
{
public:
	double *Buffer;
	unsigned int Count = 0;
	unsigned int NumOfvertex = 0;//单个顶点数据量
	ABOd(double* buffer, unsigned int NumOfvertex, unsigned int count);//将数据复制到指定位置,NumOfvertex每个顶点的数据量，count顶点个数
	~ABOd();
};
class Graphics
{
	/*坐标系是上面为Y正方向，右面为X正方向，屏幕向外为Z正方向*/
public:
	bool enable_CW=true;//是否启用顺时针逆时针三角形剔除
	bool CW_CCW = false;//默认逆时针,true为顺时针
	Matrix4 MVPMatrix;//MVP矩阵
	unsigned int Width, Height;//绘图设备宽高
	Graphics(int w, int h);
	void setVBO(VBO* vbo);
	void setTextureCoordinate(ABOd* Abo);
	void Interpolation(Point4 parry[3],double x,double y,double Weight[3]);//使用屏幕坐标插值计算三角形各个顶点的权重并保存在Weight中
	~Graphics();
	void fast_putpixel(int x, int y, COLORREF c);
	COLORREF fast_getpixel(int x, int y);
	void LoadTexture(const char* filename);//加载纹理
	void flush();// 使针对绘图窗口的显存操作生效
	void Draw();
	void clear();
	void clearDepth();//清理深度缓冲区
	void SwapS();//对于EasyX则是用BeginBatchDraw和EndBatchDraw实现的
	void SwapE();//对于EasyX则是用BeginBatchDraw和EndBatchDraw实现的
private:
	IMAGE img;
	int TextureHeight,TextureWidth;//纹理宽高
	void DrawTriangle(Point4* pointArray,Point2* textureCoordinate);//使用扫描线填充算法绘制三角形
	DWORD *g_pBuf;//显存指针
	VBO *vbo;
	ABOd *textureCoordinate;
	double* DepthBuffer = NULL;//深度缓冲区
};
#endif // !_GRAPHICS