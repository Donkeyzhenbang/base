#include <opencv2/opencv.hpp>
#include "CvxText.h"


int main(int argc, char *argv[])
{
    // 读取图像
    cv::Mat img = cv::imread("IR_20240607_162524_No18_8bit.png", cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Error: Could not load image." << std::endl;
        return -1;
    }
    
    // 创建 CvxText 对象
    CvxText text("SourceHanSerifSC-Light.otf",30); // 加载字体
    std::cout << "finish" << std::endl;
    // 设置字体属性
    CvScalar size = {15, 0.5, 0.1, 0}; // 使用 CvScalar
    float p = 0.8f; // 字体透明度
    const wchar_t *msg = L"图片测试"; // 使用宽字符
    // const char *msg = "bug xyz 2024-10-08"; // 使用宽字符
    text.setFont(NULL, &size, NULL, &p); // 设置字体

    // 使用 CvPoint 结构
    CvPoint pos = cvPoint(80, 150); 
    CvScalar s = {0, 255, 0}; //!这里有一个类型不匹配问题：cv::Scalar 不能隐式转换为 CvScalar。
    // std::cout << text.m_fontSize.val[0];
    text.putText(img, msg, pos, s); // 绘制文本

    // 保存输出图像
    imwrite("freetype.bmp", img);

    return 0;
}