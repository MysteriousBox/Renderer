#include "Vector3.h"
#include <math.h>


Vector3::Vector3(Vector3 & s, Vector3 & d) :X(d.X - s.X), Y(d.Y - s.Y), Z(d.Z - s.Z)
{
}

Vector3::Vector3(double x, double y, double z) : X(x), Y(y), Z(z)
{
}

Vector3::Vector3(Vector2 &s, double z) : X(s.X), Y(s.Y), Z(z)
{
}

void Vector3::Normalize()
{
	double mod = Mod();
	X = X / mod;
	Y = Y / mod;
	Z = Z / mod;
}

double Vector3::Mod()
{
	return sqrt(X*X + Y * Y + Z * Z);
}

Vector3 Vector3::CrossProduct(Vector3 & _a, Vector3 & _b)
{
	double l = _a.X,
		m = _a.Y,
		n = _a.Z;
	double o = _b.X,
		p = _b.Y,
		q = _b.Z;
	/**叉乘公式如下
	 * |i ,j ,k |
	 * |ax,ay,az|
	 * |bx,by,bz|
	 * |c|=|a|*|b|*sin(θ)
	 */
	return Vector3(m * q - n * p, n * o - l * q, l * p - m * o);
}

Vector3::~Vector3()
{
}
