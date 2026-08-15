#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(){
    Mat a;
    a = imread("../a.jpg");
    namedWindow("input",WINDOW_AUTOSIZE);
    namedWindow("output",WINDOW_AUTOSIZE);
    Mat new_a = Mat::zeros(a.size(),CV_8UC1);
    for(int y = 0;y!=a.rows;y++){
        for(int x = 0;x!=a.cols;x++){
            auto b = a.at<Vec3b>(y,x);
            new_a.at<uchar>(y,x) = saturate_cast<uchar>(b[0]*0.114+b[1]*0.587+b[2]*0.299);
        }
    }
    imshow("input",a);
    imshow("output",new_a);
    waitKey();
    
}