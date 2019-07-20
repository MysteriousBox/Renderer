# 大部分说明在源码中可以看见，这个项目是在尝试做软件渲染器时边想边做的，模仿了很多OpenGL的API定义
## 需要安装[easyx 下载](https://easyx.cn/)
---
使用vs直接打开EasyX.vcxproj即可  
软件直接运行可以得到如下的效果：

![运行效果](https://raw.githubusercontent.com/yangzhenzhuozz/Shader/master/result.gif)

如果想要变得更大，只需要在main.cpp中将距离拉近即可，但是因为是逐像素绘制，所以当绘制图像像素变多时，速度会相应降低
```
Vector3 eyePosition(0, 0, 10); //相机原点,将Z值变小可使得物体看起来更大
```
---
>主要实现的功能:
>> 一些简单的数学运算(矩阵相乘，向量叉乘等)  
>> 使用扫描线多边形填充算法绘制多边形  
>> 线性插值(linear interpolation)  
>> 透视校正插值(perspective correct interpolation)  
>> 深度测试(用于消隐)  
---
>一些我想到的但是暂时没实现的功能  
>> 不支持ddy和ddy，所以没有Mipmap，因为实现起来比较麻烦  
ddy和ddy是指在屏幕空间上求vbo的偏导数。真正的GPU绘制的时候是在屏幕同时绘制多个像素，GPU并行处理能力很强。  
比如当前GPU同时在屏幕上绘制四个点(x,y),(x+1,y),(x,y+1),(x+1,y+1)分别记为p1,p2,p3,p4，  
设前面三个点p(即不包含点(x+1,y+1))的纹理坐标分别为(u1,v1),(u2,v2),(u3,v3)，  
则点p1上的ddx(u)=(u2-u1)/1,ddy(u)=(u3-u1)/1,这个计算值可以用来计算Mipmap，  
同时现在主流游戏引擎上面的法线贴图都是用MikktSpace计算切线空间，也需要求偏导数，但是对我们现在的这个渲染器来说，这些可以暂时不考虑