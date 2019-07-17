# 大部分说明在源码中可以看见，这个项目是在尝试做软件渲染器时边想边做的，模仿了很多OpenGL的API定义
## 需要安装[easyx 下载](https://easyx.cn/)
---
使用vs直接打开EasyX.vcxproj即可  
软件直接运行可以得到如下的效果：

![运行效果](https://github.com/yangzhenzhuozz/Shader/blob/master/result.gif?raw=true)

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
