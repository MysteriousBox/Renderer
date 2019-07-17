#include "Vector4.h"
#include <math.h>


Vector4::Vector4():X(1),Y(1),Z(1),W(1)
{
}

Vector4::Vector4(double x, double y, double z, double w):X(x),Y(y),Z(z),W(w)
{
}

void Vector4::Normalize()
{
	double mod = Mod();
	X = X / mod;
	Y = Y / mod;
	Z = Z / mod;
	W = W / mod;
}

double Vector4::Mod()
{
	return sqrt(X*X + Y * Y + Z * Z + W * W);
}

Vector4::~Vector4()
{
}
