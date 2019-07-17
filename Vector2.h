#ifndef _Vector2
#define _Vector2
#include "Vector.h"
class Vector2:public Vector
{
public:
	double X, Y;
	Vector2(double x,double y);
	virtual double Mod();
	virtual void Normalize();//µ¥Î»»¯
	~Vector2();
};
#endif // !_Vector2

