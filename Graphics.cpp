#include "Graphics.h"
#include <list>
#include "Vector3.h"
#pragma warning(disable:4996)

Graphics::Graphics(int w, int h) :Width(w), Height(h)
{
	DepthBuffer = new double[w*h];
	initgraph(w, h);   // 创建绘图窗口，大小为 640x480 像素
	setfillcolor(RED);
	g_pBuf = GetImageBuffer(NULL);
	FragmentShader = NULL;
	VertexShader = NULL;
	TextureHeight = 0;
	TextureWidth = 0;
	TransmitAbo = NULL;
	abo = NULL;
	vbo = NULL;
}
// 快速画点函数,复制于官网教程https://codeabc.cn/yangw/post/the-principle-of-quick-drawing-points
void Graphics::fast_putpixel(int x, int y, COLORREF c)
{
	g_pBuf[y * Width + x] = BGR(c);
}

// 快速读点函数,复制于官网教程https://codeabc.cn/yangw/post/the-principle-of-quick-drawing-points
COLORREF Graphics::fast_getpixel(int x, int y)
{
	COLORREF c = g_pBuf[y * Width + x];
	return BGR(c);
}

void Graphics::LoadTexture(const char * filename)
{
	loadimage(&img, _T(filename));
	TextureHeight = img.getheight();
	TextureWidth = img.getwidth();
}

void Graphics::flush()
{
	FlushBatchDraw();
}

void Graphics::Draw()
{
	Point4 parray[3];//position Array
	for (unsigned int i = 0; i < vbo->Count / 3; i++)//i表示三角形数量
	{
		memcpy(TransmitAbo,abo->Buffer+i*3*abo->NumOfvertex,abo->NumOfvertex*sizeof(double)*3 );//将当前三角形三个顶点abo属性复制到TransmitAbo
		for (int j = 0; j < 3; j++)
		{
			VertexShader(vbo->Buffer+(i * 9 + j * 3), TransmitAbo, parray[j]);//对每个顶点调用顶点着色器
			parray[j].value[0] = parray[j].value[0] / parray[j].value[3];//X,Y,Z按照齐次坐标规则正确还原，W暂时不还原，后面插值不用1/Z，改为用1/W插值
			parray[j].value[1] = parray[j].value[1] / parray[j].value[3];
			parray[j].value[2] = parray[j].value[2] / parray[j].value[3];//经过矩阵计算,W变成了原始点的-Z值

			parray[j].value[0] = (parray[j].value[0] + 1) / 2 * Width;//将ccv空间转换到屏幕空间
			parray[j].value[1] = Height - (parray[j].value[1] + 1) / 2 * Height;
		}
		DrawTriangle(parray);
	}
}

void Graphics::clear()
{
	cleardevice();
}

void Graphics::clearDepth()
{
	memset(DepthBuffer, 0x7f, sizeof(double)*Width*Height);//用0x7f作为memset能搞定的极大值，memset应该有优化，比如调用cpu的特殊指令可以在较短的周期内赋值
}

void Graphics::SwapS()
{
	BeginBatchDraw();
}

void Graphics::SwapE()
{
	EndBatchDraw();
}

COLORREF Graphics::texture2D(double x, double y)
{
	SetWorkingImage(&img);//用于读取纹素
	COLORREF c = getpixel((int)(x*TextureWidth), TextureHeight-(int)(y*TextureHeight));
	SetWorkingImage(NULL);//恢复默认绘图设备
	return c;
}

//本函数中插值计算都是采用double
void Graphics::DrawTriangle(Point4* pArray)
{
	/*
	判断三角形的面积和方向
	*/
	Vector3 a(pArray[0].value[0] - pArray[1].value[0], pArray[0].value[1] - pArray[1].value[1], 0);
	Vector3 b(pArray[0].value[0] - pArray[2].value[0], pArray[0].value[1] - pArray[2].value[1], 0);
	//a b向量叉乘向量的z>0则表示逆时针，反之顺时针，面积不为0肯定不等于0
	//叉乘向量的模为0表示面积为0
	Vector3 t = Vector3::CrossProduct(a, b);
	if (t.Mod() == 0)
	{
		return;
	}
	if (enable_CW)
	{
		if (!CW_CCW)//顺时针
		{
			if (t.value[2] > 0)
			{
				return;
			}
		}
		else
		{
			if (t.value[2] < 0)
			{
				return;
			}
		}
	}
	unsigned int Count = 3;
	int Min = (int)pArray[0].value[1];
	int Max = (int)pArray[0].value[1];
	for (unsigned int i = 0; i < Count; i++)//记录扫描线最大最小值
	{
		if ((int)pArray[i].value[1] > Max)
		{
			Max = (int)pArray[i].value[1];
		}
		if (Min > (int)pArray[i].value[1])
		{
			Min = (int)pArray[i].value[1];
		}
	}
	std::list<EdgeTableItem> AET;//活性边表
	std::list<EdgeTableItem> *NET = new std::list<EdgeTableItem>[Max - Min + 1];//新边表 如果min=1 ,max=2 则需要 max-min+1=2-1+1行扫描线
	for (unsigned int i = 0; i < Count; i++)//对每个顶点进行扫描并添加到NET中
	{
		//Y增大方向指向屏幕下面,按照Y方向增大新增至NET和AET
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线下面，一条(pArray[i+1])在扫描线上面
		if (pArray[(i + Count - 1) % Count].value[1] > pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] < pArray[(i + Count) % Count].value[1])
		{
			NET[(int)pArray[(i + Count) % Count].value[1] - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].value[0], (pArray[(i + Count) % Count].value[0] - pArray[(i + Count - 1) % Count].value[0]) / (pArray[(i + Count) % Count].value[1] - pArray[(i + Count - 1) % Count].value[1]), pArray[(i + Count - 1) % Count].value[1]));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线上面，一条(pArray[i+1])在扫描线下面
		else if (pArray[(i + Count - 1) % Count].value[1] < pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] > pArray[(i + Count) % Count].value[1])
		{
			NET[(int)pArray[(i + Count) % Count].value[1] - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].value[0], (pArray[(i + Count) % Count].value[0] - pArray[(i + Count + 1) % Count].value[0]) / (pArray[(i + Count) % Count].value[1] - pArray[(i + Count + 1) % Count].value[1]), pArray[(i + Count + 1) % Count].value[1]));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线上面，一条(pArray[i+1])和扫描线重合
		else if (pArray[(i + Count - 1) % Count].value[1] < pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] == pArray[(i + Count) % Count].value[1])
		{
			//记录0个顶点
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])和扫描线重合，一条(pArray[i+1])在扫描线上面
		else if (pArray[(i + Count - 1) % Count].value[1] == pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] < pArray[(i + Count) % Count].value[1])
		{
			//记录0个顶点
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线下面，一条(pArray[i+1])和扫描线重合
		else if (pArray[(i + Count - 1) % Count].value[1] > pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] == pArray[(i + Count) % Count].value[1])
		{
			NET[(int)pArray[(i + Count) % Count].value[1] - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].value[0], (pArray[(i + Count) % Count].value[0] - pArray[(i + Count - 1) % Count].value[0]) / (pArray[(i + Count) % Count].value[1] - pArray[(i + Count - 1) % Count].value[1]), pArray[(i + Count - 1) % Count].value[1]));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])和扫描线重合，一条(pArray[i+1])在扫描线下面
		else if (pArray[(i + Count - 1) % Count].value[1] == pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] > pArray[(i + Count) % Count].value[1])
		{
			NET[(int)pArray[(i + Count) % Count].value[1] - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].value[0], (pArray[(i + Count) % Count].value[0] - pArray[(i + Count + 1) % Count].value[0]) / (pArray[(i + Count) % Count].value[1] - pArray[(i + Count + 1) % Count].value[1]), pArray[(i + Count + 1) % Count].value[1]));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边都在扫描线上方
		else if (pArray[(i + Count - 1) % Count].value[1] < pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] < pArray[(i + Count) % Count].value[1])
		{
			//记录0个顶点
		}
		//共享顶点pArray[(i+Count)%Count]的两条边都在扫描线下方
		else if (pArray[(i + Count - 1) % Count].value[1] > pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] > pArray[(i + Count) % Count].value[1])
		{
			NET[(int)pArray[(i + Count) % Count].value[1] - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].value[0], (pArray[(i + Count) % Count].value[0] - pArray[(i + Count - 1) % Count].value[0]) / (pArray[(i + Count) % Count].value[1] - pArray[(i + Count - 1) % Count].value[1]), pArray[(i + Count - 1) % Count].value[1]));
			NET[(int)pArray[(i + Count) % Count].value[1] - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].value[0], (pArray[(i + Count) % Count].value[0] - pArray[(i + Count + 1) % Count].value[0]) / (pArray[(i + Count) % Count].value[1] - pArray[(i + Count + 1) % Count].value[1]), pArray[(i + Count + 1) % Count].value[1]));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边都和扫描线重合
		else if (pArray[(i + Count - 1) % Count].value[1] == pArray[(i + Count) % Count].value[1]&&pArray[(i + Count + 1) % Count].value[1] == pArray[(i + Count) % Count].value[1])
		{
			//记录0个顶点
		}
		else
		{
			fprintf(stderr, "error");
		}
	}
	double* interpolationAbo=new double[abo->NumOfvertex];//插值之后的ABO
	for (int i = Min; i < Max; i++)//开始绘制
	{
		std::list<EdgeTableItem>::iterator it_end = AET.end();
		AET.splice(it_end, NET[i - Min]);
		AET.sort([](EdgeTableItem const & E1, EdgeTableItem const &E2) {return E1.x < E2.x; });//排序
		std::list<EdgeTableItem>::iterator s, e;
		int CountPosite = 0;
		for (std::list<EdgeTableItem>::iterator it = AET.begin(); it != AET.end();)
		{
			if ((int)it->Ymax <= i)
			{
				it = AET.erase(it);//当前扫描线已经超过it这条边的Ymax,将it边删除
			}
			else
			{
				CountPosite++;
				if (CountPosite % 2 == 1)//起始顶点
				{
					s = it;
				}
				else
				{
					e = it;
					if (i >= 0 && i < (int)Height)//只绘制出现在屏幕范围之内的像素
					{
						for (unsigned int j = max((int)s->x,0); j < min(e->x,Width); j++)
						{
							if (j >= 0 && j < Width)
							{
								double Weight[3] = {0,0,0};
								Interpolation(pArray, j, i, Weight);//使用重心坐标插值计算出三个顶点对(j,i)的权重
								double depth = Weight[0] * pArray[0].value[2] + Weight[1] * pArray[1].value[2] + Weight[2] * pArray[2].value[2];//计算深度值，这个值虽然不是线性的，但是经过线性插值仍然能保证大的更大，小的更小
								/*
								 普通线性插值计算出(j,i)的值:v=Weight[0]*v1+Weight[1]*v2+Weight[2]*v3
								 深度值Depth:D(j,i)=1/z=Weight[0]*(1/z1)+Weight[1]*(1/z2)+Weight[2]*(1/z3)
								 根据透视校正的原理(j,i)的值:v/z=Weight[0]*(v1/z1)+Weight[1]*(v2/z2)+Weight[2]*(v3/z3)
								*/
								double originDepth = 1 / (Weight[0] * (1 / pArray[0].value[3]) + Weight[1] * (1 / pArray[1].value[3]) + Weight[2] * (1 / pArray[2].value[3]));//这个值是原始深度
								for (unsigned int index=0;index<abo->NumOfvertex;index++)//对每个abo插值
								{
									interpolationAbo[index] = originDepth * (   TransmitAbo[index]  / pArray[0].value[3] * Weight[0] + TransmitAbo[index+abo->NumOfvertex] / pArray[1].value[3] * Weight[1] + TransmitAbo[index + abo->NumOfvertex*2] / pArray[2].value[3] * Weight[2]);
								}
								if (DepthBuffer[i*Width + j] > depth)//因为在perspective Matrix中取反，所以应该是值越小则近
								{
									COLORREF c;
									FragmentShader(interpolationAbo,c);//调用片元着色器
									fast_putpixel(j, i, c);//先用屏幕空间重心插值求出纹理(暂时不加透视校正) i 扫描线序号，j横坐标序号
									DepthBuffer[i*Width + j] = depth;//更新深度信息
								}
							}
						}
					}

					s->x += s->dx;//更新NET
					e->x += e->dx;
				}
				++it;
			}
		}
	}
	delete[] NET;
	delete[] interpolationAbo;
}

void Graphics::setVBO(VBO *Vbo)
{
	vbo = Vbo;
}

void Graphics::setABO(ABO *Abo)
{
	if (TransmitAbo != NULL)
	{
		delete TransmitAbo;
	}
	TransmitAbo = new double[Abo->NumOfvertex*3];
	abo = Abo;
}

void Graphics::Interpolation(Point4 pArray[3], double x, double y, double Weight[3])
{
	/*
	两点式：
	  x-X1      y-Y1
	------- =  -------
	 X2-X1      Y2-Y1
	 转化为一般式:
	 Ax+By+C=0
	 Ax+By=-C
	 A=Y2-Y1
	 B=X1-X2
	 -C=X1*Y2-X2*Y1
	*/
	for (int i=0;i<3;i++)
	{
		if (x == pArray[i].value[0] && y == pArray[0].value[1])
		{
			Weight[i] = 1;//和顶点重合，权重为1
			return;
		}
	}
	double A11 = pArray[2].value[1] - pArray[1].value[1];
	double A12 = pArray[1].value[0] - pArray[2].value[0];
	double B1 = pArray[2].value[1]*pArray[1].value[0] - pArray[1].value[1]*pArray[2].value[0];//得到P2 P1直线方程

	double A21 = pArray[0].value[1] - y;
	double A22 = x - pArray[0].value[0];
	double B2 = pArray[0].value[1]*x - y * pArray[0].value[0];//得到P0 (x,y)直线方程

	//除非三角形的两条边平行，否则有解,用克莱姆法则即可求出交点(X,Y)
	if ((A11 * A22 - A12 * A21) == 0)//P0 (x,y)直线和P1 P2直线平行，无交点，权重取P0
	{
		Weight[0] = 1;
		Weight[1] = 0;
		Weight[2] = 0;
		return;
	}
	double X = (B1*A22 - A12 * B2) / (A11*A22 - A12 * A21);
	double Y = (B2*A11 - A21 * B1) / (A11*A22 - A12 * A21);

	double W0, W1, W2, Wt;
	/*
	求出(X,Y)对应的插值
	*/
	if (pArray[2].value[1] - pArray[1].value[1] != 0)
	{
		W1 = (pArray[2].value[1] - Y) / (pArray[2].value[1] - pArray[1].value[1]);//P[2]权重 weight
		W2 = (Y - pArray[1].value[1]) / (pArray[2].value[1] - pArray[1].value[1]);//P[2]权重 weight
	}
	else
	{
		W1 = (pArray[2].value[0] - X) / (pArray[2].value[0] - pArray[1].value[0]);//P[2]权重 weight
		W2 = (X - pArray[1].value[0]) / (pArray[2].value[0] - pArray[1].value[0]);//P[2]权重 weight
	}
	if (pArray[0].value[1] - Y != 0)
	{
		Wt = (y - pArray[0].value[1]) / (Y - pArray[0].value[1]);
		W0 = (Y - y) / (Y - pArray[0].value[1]);
	}
	else
	{
		Wt = (x - pArray[0].value[0]) / (X - pArray[0].value[0]);
		W0 = (X - x) / (X - pArray[0].value[0]);
	}
	Weight[0] = W0;
	Weight[1] = W1 * Wt;
	Weight[2] = W2 * Wt;
}



Graphics::~Graphics()
{
	if (DepthBuffer != NULL)
	{
		delete DepthBuffer;
	}
	if (TransmitAbo != NULL)
	{
		delete TransmitAbo;
	}
	closegraph();          // 关闭绘图窗口
}

VBO::VBO(double * buffer, unsigned int count) :Count(count)
{
	Buffer = new double[count * 3];
	memcpy(Buffer, buffer, sizeof(double)*count * 3);
}

VBO::~VBO()
{
	delete Buffer;
}

ABO::ABO(double * buffer, unsigned int numOfVertex, unsigned int count) :Count(count)
{
	Buffer = new double[count* numOfVertex];
	memcpy(Buffer, buffer, sizeof(double)*count* numOfVertex);
	NumOfvertex = numOfVertex;
}

ABO::~ABO()
{
	delete Buffer;
}
