#ifndef  _Vector
#define _Vector
class Vector
{
public:
	Vector();
	virtual double Mod() = 0;//求模运算
	virtual void Normalize() = 0;//单位化
	virtual ~Vector();
};
#endif // ! _Vector

