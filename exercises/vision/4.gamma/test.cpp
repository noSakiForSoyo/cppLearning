#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
int main(int argc,char *argv[]){
    Mat a;
    int gamma;
    std::cout << "请输入伽马值" << '\n';
    std::cin >> gamma;
    a = imread("../a.jpg");
    namedWindow("output",WINDOW_AUTOSIZE);
    Mat lookUpTO(1,256,CV_8U);
    uchar *p = lookUpTO.ptr();
    for(int i = 0;i != 256;++i){
        p[i] = saturate_cast<uchar>(pow(i/255.0,gamma)*255.0);
    }
    Mat m = a.clone();
    LUT(a,lookUpTO,m);
    imshow("output",m);
    waitKey();
}