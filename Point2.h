#ifndef _Point2
#define _Point2
class Point2//复制构造和赋值用浅拷贝目前够了
{
public:
	double X, Y;
	Point2();
	Point2(double x, double y);
	~Point2();
};
#endif // !_POINT

