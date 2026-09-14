#include <opencv2/opencv.hpp>
using namespace cv;
int main() {
    Mat img = Mat::zeros(100, 100, CV_8UC1);
    imshow("hello opencv", img);
    waitKey(0);
    return 0;
}