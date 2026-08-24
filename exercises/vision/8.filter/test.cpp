#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
Mat src;
int sigmaX = 0;
int sigmaColor = 1;
int max_size = 100;
int ksize = 1;
int d = 5;
void gaussianByAPI(int , void*);
void bilateralByAPI(int , void*);
void median();

int main(){

    src = imread("../b.jpg",IMREAD_GRAYSCALE);

    namedWindow("gaussian",WINDOW_AUTOSIZE);
    namedWindow("bilateral",WINDOW_AUTOSIZE);
    namedWindow("median",WINDOW_AUTOSIZE);
    namedWindow("src",WINDOW_AUTOSIZE);

    createTrackbar("高斯滤波X","gaussian",&sigmaX,max_size,gaussianByAPI);
    createTrackbar("高斯滤波ksize","gaussian",&ksize,max_size,gaussianByAPI);
    createTrackbar("双边滤波color","bilateral",&sigmaColor,max_size,bilateralByAPI);
    createTrackbar("双边滤波d","bilateral",&d,max_size,bilateralByAPI);
    median();
    imshow("src",src);
    waitKey();
}

void gaussianByAPI(int , void*){
    Mat dst1(src.size(),src.type());
    GaussianBlur(src,dst1,Size(ksize*2+1,ksize*2+1),sigmaX);
    imshow("gaussian",dst1);
}
void bilateralByAPI(int , void*){
    Mat dst2(src.size(),src.type());
    bilateralFilter(src,dst2,d,sigmaColor,sigmaColor);
    imshow("bilateral",dst2);
}
void median(){
    Mat dst3(src.size(),src.type());
    medianBlur(src,dst3,3);
    imshow("median",dst3);
}