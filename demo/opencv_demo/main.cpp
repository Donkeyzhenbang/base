#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 读取图片
    cv::Mat image = cv::imread("home.png");
    
    // 检查图片是否成功读取
    if (image.empty()) {
        std::cerr << "Could not open or find the image!" << std::endl;
        return -1;
    }

    // 显示图片
    cv::imshow("Display Image", image);

    // 等待按键事件，防止窗口立即关闭
    cv::waitKey(10000);

    return 0;
}
