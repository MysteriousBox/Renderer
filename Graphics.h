#ifndef _GRAPHICSM
#define _GRAPHICSM
#include <graphics.h>      // 引用图形库头文件
#include <list>
#include "Point2.h"
#include "Point4.h"
#include "EdgeTableItem.h"
#include "Matrix4.h"
/*
不支持ddy和ddy，所以没有Mipmap，因为实现起来比较麻烦
ddy和ddy是指在屏幕空间上求vbo的偏导数
真正的GPU绘制的时候是在屏幕同时绘制多个像素，GPU并行处理能力很强
比如当前GPU同时在屏幕上绘制四个点(x,y),(x+1,y),(x,y+1),(x+1,y+1)分别记为p1,p2,p3,p4
设前面三个点p(即不包含点(x+1,y+1))的纹理坐标分别为(u1,v1),(u2,v2),(u3,v3)
则点p1上的ddx(u)=(u2-u1)/1,ddy(u)=(u3-u1)/1,这个计算值可以用来计算Mipmap
同时现在主流游戏引擎上面的法线贴图都是用MikktSpace计算切线空间，也需要求偏导数，但是对我们现在的这个渲染器来说，这些可以暂时不考虑

返回bool的函数表示有成功或者失败的问题，如果执行失败，可以通过errmsg读取，任何返回bool的函数在进入的时候都会清空errmsg，所以一旦出现问题，请立马读取errmsg

坐标系是上面为Y正方向,右面为X正方向,屏幕向外为Z正方向

视口空间和屏幕空间的宽高单位一样，仅仅是增加了一个x,y的偏移
*/
class Graphics
{
	/*坐标系是上面为Y正方向，右面为X正方向，屏幕向外为Z正方向*/
public:
	bool setViewPort(int x,int y,int w,int h);//设置视口坐标和宽高，即在屏幕内部的一个子区域绘制
	char errmsg[1024] = { '0' };//错误信息，如果执行出错则可以读取本信息
	void (*VertexShader)(double const vbo[3], double* abo, double* varying, Point4& Position);//顶点着色器，概念和OpenGL类似，但是参数有区别，下面是ABO的说明
	/*
	Position对应了OpenGL的gl_Position，
	VBO来当前顶点的xyz，来自vbo，
	abo来当前顶点的abo，来自abo
	varying表示需要通过顶点着色器传递给片元着色器的变量，会在顶点处插值被传递给片元着色器
	本程序需要计算并设置当abo和Position
	*/
	void (*FragmentShader)(double* ABO, double* varying, COLORREF& FragColor);//片源，概念和OpenGL类似，但是参数有区别，下面是ABO的说明
	/*
	FragColor对应于OpenGL的gl_FragColor，只是没有了透明度，只有RGB
	ABO在经过1/w插值之后会传递给片元着色器
	顶点着色器中的Varying经过1/w插值之后传递给片元着色器
	本程序需要计算并设置当abo和Position
	*/
	bool enable_CW = true;//是否启用顺时针逆时针三角形剔除
	bool CW_CCW = false;//默认逆时针,true为顺时针
	Graphics(unsigned int w, unsigned int h);
	void setVBO(double* buffer, int count);
	void setABO(double* buffer, int numOfvertex, int count);
	void Interpolation(Point4 parry[3], double x, double y, double Weight[3]);//使用屏幕坐标插值计算三角形各个顶点的权重并保存在Weight中
	~Graphics();
	void fast_putpixel(int x, int y, COLORREF c);
	COLORREF fast_getpixel(int x, int y);
	bool loadBMP(const char* filename);//加载bmp文件到纹理,返回false表示失败（只能加载24位，无压缩，纵轴正向BMP）
	void flush();// 使针对绘图窗口的显存操作生效
	bool Draw();//返回false表示绘制失败
	void clear();
	void clearDepth(double v);//清理深度缓冲区,注意这里是使用memset来填充的，所以假如c=0x01，对应像素的深度值是一个dobule类型8字节,则实际上会被填充成0x0101010101010101
	void SwapStart();//对于EasyX则是用BeginBatchDraw和EndBatchDraw实现的
	void SwapEnd();//对于EasyX则是用BeginBatchDraw和EndBatchDraw实现的
	void setVaryingCount(int count);//设置Varying变量的数量，如果有使用Varying，则一定要调用本函数
	COLORREF texture2D(double x, double y);//读取纹理中的颜色
private:
	unsigned int ScreenWidth = 0, ScreenHeight = 0;
	unsigned viewPortX = 0, viewPortY = 0, viewPortWidth = 0, viewPortHeight = 0;//视口起始坐标和宽高，从左下角开始
	int bmpHeight = 0;
	int bmpwidth = 0;//位图宽高
	unsigned char* bmpData = NULL;//位图数据区
	unsigned char* textureBuffer = NULL;//纹理缓冲区，保存bmp位图
	double* TransmitAbo;//从顶点着色器传递到片元着色器的ABO
	int TextureHeight, TextureWidth;//纹理宽高
	void DrawTriangle(Point4* pointArray);//使用扫描线填充算法绘制三角形
	DWORD* g_pBuf;//显存指针
	double* DepthBuffer = NULL;//深度缓冲区
	double* vboBuffer = NULL;//vob
	int vboCount = 0;//顶点数量
	double* aboBuffer = NULL;//abo
	int NumOfVertexABO = 0;//每个顶点的属性数量
	int aboCount = 0;//abo数量
	double* Varying = NULL;//当前的Varying变量，经过插值之后会传递给片元着色器
	int CountOfVarying = 0;//Varying变量数量
	std::list<EdgeTableItem>* NET = NULL;//新边表和ViewPortHeight大小一样
	double* interpolationAbo = NULL;//当前线程在绘制当前顶点插值之后的ABO，因为单线程，所以这里只需要一个就行了
	double* interpolationVarying = NULL;//当前线程在绘制当前顶点插值之后的varying，因为单线程，所以这里只需要一个就行了
};
#endif // !_GRAPHICSM