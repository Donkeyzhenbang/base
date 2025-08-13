### freetype移植
#### 类型匹配
```cpp
CvScalar s = {0, 255, 0}; //!这里有一个类型不匹配问题：cv::Scalar 不能隐式转换为 CvScalar。
text.putText(img, msg, pos, s); // 绘制文本
```
#### 中文字体使用宽字符
```cpp
const wchar_t *msg = L"偏振图像"; // 使用宽字符
```
#### CvxText
- 源文件有问题 注意修改 源码地址https://bigbookplus.github.io/blog/2022-06-15-cvxtext-opencv45.html
- 注意linux引用opencv 使用/
- Windows使用\
- 如下图，注意临时对象不能右值引用，Visual Studio中会将其延长生命周期 linux-g++编译不会优化编译
```cpp
void CvxText::putWChar(cv::Mat &frame, wchar_t wc, CvPoint &pos, CvScalar color)
{
	// IplImage* img = NULL;
	// img = &(cvIplImage(frame));
	//!临时对象非const不能右值引用
	IplImage temp_img = cvIplImage(frame);  // 创建IplImage的副本
    IplImage* img = &temp_img;              // 取得指针
}
```

#### Makefile
- 只需要freetype2.pc 
- 注意看系统中是否有freetype的动态库 避免多个库复用导致vim出现问题 我的Ubuntu使用的freetype动态链接库版本是libfreetype.so.6.17.1
- 之前在Makefile中必须要加上`LDFLAGS += -L/usr/lib/x86_64-linux-gnu/`这句话是因为在移植freetype时执行sudo make install所以将另一个版本的pc文件或者动态/静态库安装在/usr/local/lib中 删除即可

#### msyh.ttf
- 微软雅黑字体msyh.ttf需放到运行目录下