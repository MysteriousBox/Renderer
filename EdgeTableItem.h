#ifndef _EdgeTable
#define _EdgeTable
#include <stdio.h>
class EdgeTableItem//活性边表项
{
public:
	double x=0, dx = 0, Ymax = 0;
	EdgeTableItem(double X,double DX,double YMAX);
	~EdgeTableItem();
};
#endif // !_ActiveEdgeTable

