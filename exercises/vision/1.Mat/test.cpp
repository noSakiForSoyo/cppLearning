#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(){
    cv::Mat a,c;
    a = cv::imread("../a.jpg");
    Rect r(10,10,11,11);
    Mat smallImg = a(r);
    cvtColor(a,c,COLOR_BGR2GRAY);
    namedWindow("image",WINDOW_AUTOSIZE);
    imshow("image",smallImg);
    waitKey();
}