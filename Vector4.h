#ifndef _Vector4
#define _Vector4
#include "Vector.h"
class Vector4 :
	public Vector
{
public:
	double X, Y, Z, W;
	Vector4();
	Vector4(double x, double y, double z, double w);
	virtual void Normalize();
	virtual double Mod();
	~Vector4();
};
#endif // !_Vector4

