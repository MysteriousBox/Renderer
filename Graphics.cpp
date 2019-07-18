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
	Point2 carray[3];//Coordinate Array
	for (unsigned int i = 0; i < vbo->Count / 3; i++)//i表示三角形数量
	{
		for (int j = 0; j < 3; j++)
		{
			double src[4];
			src[0] = vbo->Buffer[i * 9 + j * 3];
			src[1] = vbo->Buffer[i * 9 + j * 3 + 1];
			src[2] = vbo->Buffer[i * 9 + j * 3 + 2];
			src[3] = 1;
			double dest[4];
			Matrix::Mult(MVPMatrix.Value[0], src, 4, 1, 4, dest);//类似于Vertex shader的功能
			
			parray[j].X = dest[0] / dest[3];//X,Y,Z按照齐次坐标规则正确还原，W暂时不还原，后面插值不用1/Z，改为用1/W插值
			parray[j].Y = dest[1] / dest[3];
			parray[j].Z = dest[2] / dest[3];
			parray[j].W = dest[3];//经过矩阵计算,W变成了原始点的-Z值


			parray[j].X = (parray[j].X + 1) / 2 * Width;//将ccv空间转换到屏幕空间
			parray[j].Y = Height - (parray[j].Y + 1) / 2 * Height;



			carray[j].X = textureCoordinate->Buffer[i * 6 + j * 2];//纹理坐标转换成纹素坐标
			carray[j].Y = textureCoordinate->Buffer[i * 6 + j * 2 + 1];
		}
		DrawTriangle(parray, carray);
	}
}

void Graphics::clear()
{
	cleardevice();
}

void Graphics::clearDepth()
{
	memset(DepthBuffer, 0x7f,sizeof(double)*Width*Height);//用0x7f作为memset能搞定的极大值，memset应该有优化，比如调用cpu的特殊指令可以在较短的周期内赋值
}

void Graphics::SwapS()
{
	BeginBatchDraw();
}

void Graphics::SwapE()
{
	EndBatchDraw();
}

//本函数中插值计算都是采用double
void Graphics::DrawTriangle(Point4* pArray, Point2* textureCoordinate)
{
	/*
	判断三角形的面积和方向
	*/
	Vector3 a(pArray[0].X - pArray[1].X, pArray[0].Y - pArray[1].Y, 0);
	Vector3 b(pArray[0].X - pArray[2].X, pArray[0].Y - pArray[2].Y, 0);
	//a b向量叉乘向量的z>0则表示逆时针，反之顺时针，面积不为0肯定不等于0
	//叉乘向量的模为0表示面积为0
	Vector3 t=Vector3::CrossProduct(a, b);
	if (t.Mod() == 0)
	{
		return;
	}
	if (t.Z == 0)
	{
		OutputDebugString("错误");
		return;
	}
	if (enable_CW)
	{
		if (!CW_CCW)//顺时针
		{
			if (t.Z > 0)
			{
				return;
			}
		}
		else
		{
			if (t.Z < 0)
			{
				return;
			}
		}
	}




	unsigned int Count = 3;
	int Min = (int)pArray[0].Y;
	int Max = (int)pArray[0].Y;
	for (unsigned int i = 0; i < Count; i++)//记录最大最小值
	{
		if ((int)pArray[i].Y > Max)
		{
			Max = (int)pArray[i].Y;
		}
		if (Min > (int)pArray[i].Y)
		{
			Min = (int)pArray[i].Y;
		}
	}
	std::list<EdgeTableItem> AET;//活性边表
	std::list<EdgeTableItem> *NET = new std::list<EdgeTableItem>[Max - Min+1];//新边表 如果min=1 ,max=2 则需要 max-min+1=2-1+1行扫描线
	for (unsigned int i = 0; i < Count; i++)//对每个顶点进行扫描并添加到NET中
	{
		//Y增大方向指向屏幕下面,按照Y方向增大新增至NET和AET
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线下面，一条(pArray[i+1])在扫描线上面
		if (pArray[(i + Count - 1) % Count].Y > pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y < pArray[(i + Count) % Count].Y)
		{
			NET[(int)pArray[(i + Count) % Count].Y - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].X, (pArray[(i + Count) % Count].X - pArray[(i + Count - 1) % Count].X) / (pArray[(i + Count) % Count].Y - pArray[(i + Count - 1) % Count].Y), pArray[(i + Count - 1) % Count].Y));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线上面，一条(pArray[i+1])在扫描线下面
		else if (pArray[(i + Count - 1) % Count].Y < pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y > pArray[(i + Count) % Count].Y)
		{
			NET[(int)pArray[(i + Count) % Count].Y - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].X, (pArray[(i + Count) % Count].X - pArray[(i + Count + 1) % Count].X) / (pArray[(i + Count) % Count].Y - pArray[(i + Count + 1) % Count].Y), pArray[(i + Count + 1) % Count].Y));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线上面，一条(pArray[i+1])和扫描线重合
		else if (pArray[(i + Count - 1) % Count].Y < pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y == pArray[(i + Count) % Count].Y)
		{
			//记录0个顶点
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])和扫描线重合，一条(pArray[i+1])在扫描线上面
		else if (pArray[(i + Count - 1) % Count].Y == pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y < pArray[(i + Count) % Count].Y)
		{
			//记录0个顶点
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])在扫描线下面，一条(pArray[i+1])和扫描线重合
		else if (pArray[(i + Count - 1) % Count].Y > pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y == pArray[(i + Count) % Count].Y)
		{
			NET[(int)pArray[(i + Count) % Count].Y - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].X, (pArray[(i + Count) % Count].X - pArray[(i + Count - 1) % Count].X) / (pArray[(i + Count) % Count].Y - pArray[(i + Count - 1) % Count].Y), pArray[(i + Count - 1) % Count].Y));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边一条(pArray[i-1])和扫描线重合，一条(pArray[i+1])在扫描线下面
		else if (pArray[(i + Count - 1) % Count].Y == pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y > pArray[(i + Count) % Count].Y)
		{
			NET[(int)pArray[(i + Count) % Count].Y - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].X, (pArray[(i + Count) % Count].X - pArray[(i + Count + 1) % Count].X) / (pArray[(i + Count) % Count].Y - pArray[(i + Count + 1) % Count].Y), pArray[(i + Count + 1) % Count].Y));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边都在扫描线上方
		else if (pArray[(i + Count - 1) % Count].Y < pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y < pArray[(i + Count) % Count].Y)
		{
			//记录0个顶点
		}
		//共享顶点pArray[(i+Count)%Count]的两条边都在扫描线下方
		else if (pArray[(i + Count - 1) % Count].Y > pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y > pArray[(i + Count) % Count].Y)
		{
			NET[(int)pArray[(i + Count) % Count].Y - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].X, (pArray[(i + Count) % Count].X - pArray[(i + Count - 1) % Count].X) / (pArray[(i + Count) % Count].Y - pArray[(i + Count - 1) % Count].Y), pArray[(i + Count - 1) % Count].Y));
			NET[(int)pArray[(i + Count) % Count].Y - Min].push_back(EdgeTableItem(pArray[(i + Count) % Count].X, (pArray[(i + Count) % Count].X - pArray[(i + Count + 1) % Count].X) / (pArray[(i + Count) % Count].Y - pArray[(i + Count + 1) % Count].Y), pArray[(i + Count + 1) % Count].Y));
		}
		//共享顶点pArray[(i+Count)%Count]的两条边都和扫描线重合
		else if (pArray[(i + Count - 1) % Count].Y == pArray[(i + Count) % Count].Y&&pArray[(i + Count + 1) % Count].Y == pArray[(i + Count) % Count].Y)
		{
			//记录0个顶点
		}
		else
		{
			fprintf(stderr, "error");
		}
	}
	for (int i = Min; i < Max; i++)
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
						for (unsigned int j = (int)s->x; j < e->x; j++)
						{
							if (j >= 0 && j < Width)
							{
								double Weight[3];
								double coordinate_X, coordinate_Y;
								Interpolation(pArray, j, i, Weight);//使用重心坐标插值计算出三个顶点对(j,i)的权重
								double depth = Weight[0] * pArray[0].Z + Weight[1] * pArray[1].Z + Weight[2] * pArray[2].Z;//计算深度值，这个值虽然不是线性的，但是经过线性插值仍然能保证大的更大，小的更小
								/*
								 普通线性插值计算出(j,i)的值:v=Weight[0]*v1+Weight[1]*v2+Weight[2]*v3
								 深度值Depth:D(j,i)=1/z=Weight[0]*(1/z1)+Weight[1]*(1/z2)+Weight[2]*(1/z3)
								 根据透视校正的原理(j,i)的值:v/z=Weight[0]*(v1/z1)+Weight[1]*(v2/z2)+Weight[2]*(v3/z3)
								*/
								double originDepth = 1/(Weight[0] * (1 / pArray[0].W) + Weight[1] * (1 / pArray[1].W) + Weight[2] * (1 / pArray[2].W));//这个值是原始深度

								coordinate_X = originDepth * (textureCoordinate[0].X / pArray[0].W * Weight[0] + textureCoordinate[1].X / pArray[1].W * Weight[1] + textureCoordinate[2].X / pArray[2].W * Weight[2])* TextureWidth;
								coordinate_Y = originDepth * (textureCoordinate[0].Y / pArray[0].W * Weight[0] + textureCoordinate[1].Y / pArray[1].W * Weight[1] + textureCoordinate[2].Y / pArray[2].W * Weight[2])* TextureHeight;

								if (coordinate_X > TextureWidth)
								{
									coordinate_X = TextureWidth;
								}
								else if (coordinate_X < 0)
								{
									coordinate_X = 0;
								}
								if (coordinate_Y > TextureWidth)
								{
									coordinate_Y = TextureHeight;
								}
								else if (coordinate_Y < 0)
								{
									coordinate_Y = 0;
								}
								SetWorkingImage(&img);//用于读取纹素
								COLORREF c = getpixel((int)coordinate_X, (int)(TextureHeight - coordinate_Y));
								SetWorkingImage(NULL);//恢复默认绘图设备
								if (DepthBuffer[i*Width+j]>depth)//因为在perspective Matrix中取反，所以应该是值越小则近
								{
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
}

void Graphics::setVBO(VBO *Vbo)
{
	vbo = Vbo;
}

void Graphics::setTextureCoordinate(ABOd *Abo)
{
	textureCoordinate = Abo;
}

void Graphics::Interpolation(Point4 pArray[3], double x, double y, double Weight[3])
{
	/*
	两点式：
	  x-X1      y-Y1
	------- =  -------
	 X2-X1      Y2-Y1
	 转化为一般式:
	 (Y2-Y1)*x-(X2-X1)*y-X1*(Y2-Y1)+Y1*(X2-X1)=0
	 (Y2-Y1)*x-(X2-X1)*y+Y1*X2-Y2*X1=0
	 (Y2-Y1)*x-(X2-X1)*y=Y2*X1-Y1*X2
	 A1=Y2-Y1
	 A2=X1-X2
	 B=Y2*X1-Y1*X2
	*/
	double A11 = pArray[2].Y - pArray[1].Y;
	double A12 = pArray[1].X - pArray[2].X;
	double B1 = pArray[2].Y*pArray[1].X - pArray[1].Y*pArray[2].X;

	double A21 = pArray[0].Y - y;
	double A22 = x - pArray[0].X;
	double B2 = pArray[0].Y*x - y * pArray[0].X;

	//除非三角形的两条边平行，否则有解,用克莱姆法则即可求出交点(X,Y)
	double X = (B1*A22 - A12 * B2) / (A11*A22 - A12 * A21);
	double Y = (B2*A11 - A21 * B1) / (A11*A22 - A12 * A21);

	double W0, W1, W2, Wt;
	/*
	求出(X,Y)对应的插值
	*/
	if (pArray[2].Y - pArray[1].Y != 0)
	{
		W1 = (pArray[2].Y - Y) / (pArray[2].Y - pArray[1].Y);//P[2]权重 weight
		W2 = (Y - pArray[1].Y) / (pArray[2].Y - pArray[1].Y);//P[2]权重 weight
	}
	else
	{
		W1 = (pArray[2].X - X) / (pArray[2].X - pArray[1].X);//P[2]权重 weight
		W2 = (X - pArray[1].X) / (pArray[2].X - pArray[1].X);//P[2]权重 weight
	}
	if (pArray[0].Y - Y != 0)
	{
		Wt = (y - pArray[0].Y) / (Y - pArray[0].Y);
		W0 = (Y - y) / (Y - pArray[0].Y);
	}
	else
	{
		Wt = (x - pArray[0].X) / (X - pArray[0].X);
		W0 = (X - x) / (X - pArray[0].X);
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

ABOd::ABOd(double * buffer, unsigned int NumOfVertex, unsigned int count) :Count(count)
{
	Buffer = new double[count*NumOfVertex];
	memcpy(Buffer, buffer, sizeof(double)*count*NumOfVertex);
}

ABOd::~ABOd()
{
	delete Buffer;
}
