#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

Mat src, src_blur, src_gray;
Mat dst, edges;
int min_threshold = 0;
const int ratio = 3;
const std::string win = "dst";
const int kernel_size = 3;
const int max_min_threshold = 100;
void cannyByApi(int, void *)
{
    blur(src_gray, src_blur, Size(3, 3));
    Canny(src_blur, edges, min_threshold, min_threshold * ratio, kernel_size);
    dst = Scalar::all(0);
    src.copyTo(dst, edges);
    imshow(win, dst);
}

int main()
{
    VideoCapture cap(1);
    if (!cap.isOpened())
    {
        std::cerr << "摄像头打开失败";
        return -1;
    }
    namedWindow(win, WINDOW_AUTOSIZE);
    createTrackbar("threshold", win, &min_threshold, max_min_threshold, cannyByApi);
    while (cap.read(src))
    {
        std::cout << "ss";
        cvtColor(src, src_gray, COLOR_BGR2GRAY);
        cannyByApi(min_threshold, 0);
        if (waitKey(30) == 27)
            break;
    }
    return 0;
}