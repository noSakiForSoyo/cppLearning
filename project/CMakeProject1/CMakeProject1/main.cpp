#include <iostream>
#include <opencv2/opencv.hpp>

int main()
{
    cv::Mat img = cv::imread("test.jpg");   // 读一张图片（把任意 jpg 放到 exe 旁边）
    if (img.empty()) {
        std::cout << "图片没读到，检查 test.jpg 放哪了" << std::endl;
        return -1;
    }
    cv::imshow("我的第一个 OpenCV 窗口", img);
    cv::waitKey(0);   // 按任意键关闭
    return 0;
}