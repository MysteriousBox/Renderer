#include "Vector2.h"
#include <math.h>

Vector2::Vector2(double x, double y) :X(x), Y(y)
{
}

Vector2::~Vector2()
{
}

double Vector2::Mod()
{
	return sqrt(X*X + Y * Y);
}

void Vector2::Normalize()
{
	double mod = Mod();
	X = X / mod;
	Y = Y / mod;
}
