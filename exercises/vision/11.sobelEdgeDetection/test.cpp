#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
Mat src,src_gua,src_gray,dst;
Mat gradX,gradY;
Mat AbsGradX,AbsGradY;
int ksize=0;
int maxsize = 100;
int ddepth = CV_16S;
int scale=1,delta=0;
std::string str;

void sobelByApi(int ,void *);
int main(int argc,char *argv[]){
    src = imread("../a.jpg");
    if(src.empty()){
        std::cerr << "error";
        return -1;
    }
        std::cout <<"this\n";
    GaussianBlur(src,src_gua,Size(3,3),0,0);
    cvtColor(src_gua,src_gray,COLOR_BGR2GRAY);
    std::cout <<"this\n";
    namedWindow("dst",WINDOW_AUTOSIZE);
    namedWindow("gradX",WINDOW_AUTOSIZE);
    namedWindow("gradY",WINDOW_AUTOSIZE);
    createTrackbar("ksize","dst",&ksize,10,sobelByApi);
    createTrackbar("scale","dst",&scale,maxsize,sobelByApi);
    createTrackbar("delta","dst",&delta,maxsize,sobelByApi);
    waitKey();
}

void sobelByApi(int ,void *){
    Sobel(src_gray,gradX,ddepth,1,0,ksize*2+1,scale,delta);
    Sobel(src_gray,gradY,ddepth,0,1,ksize*2+1,scale,delta);
    convertScaleAbs(gradX,AbsGradX);
    convertScaleAbs(gradY,AbsGradY);
    addWeighted(AbsGradX,0.5,AbsGradY,0.5,0.0,dst);
    imshow("dst",dst);
    imshow("gradX",gradX);
    imshow("gradY",gradY);
}