#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
Mat drawHist(const std::array<int,256>& arr,const int &height);
std::array<int ,256> getArr(const Mat &src);
void otsu(const Mat &src,Mat &dst);
int main(){
    Mat src = imread("../b.jpg",IMREAD_GRAYSCALE);
    if(src.empty()){
        std::cerr << "文件未打开";
        return -1;
    }
    Mat dst(src.size(),src.type());   
    Mat dst1(src.size(),src.type());
    Mat dst2(src.size(),src.type());
    Mat dst3(src.size(),src.type());
    Mat dst4(src.size(),src.type());    
    Mat dst5(src.size(),src.type());   
    auto arr = getArr(src);
    Mat hist = drawHist(arr,300);
    otsu(src,dst);
    double t =  threshold(src, dst1, 0, 255, THRESH_BINARY | THRESH_OTSU);
    std::cout << t <<"\n";
    threshold(src, dst2, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
    threshold(src, dst3, 0, 255, THRESH_TRUNC | THRESH_OTSU);
    threshold(src, dst4, 0, 255, THRESH_TOZERO | THRESH_OTSU);
    threshold(src, dst5, 0, 255, THRESH_TOZERO_INV | THRESH_OTSU);
    namedWindow("hist",WINDOW_AUTOSIZE);
    namedWindow("dst1",WINDOW_AUTOSIZE);
    namedWindow("dst2",WINDOW_AUTOSIZE);
    namedWindow("dst3",WINDOW_AUTOSIZE);
    namedWindow("dst4",WINDOW_AUTOSIZE);
    namedWindow("dst5",WINDOW_AUTOSIZE);
    imshow("hist",hist);
    imshow("otsu",dst);
    imshow("dst1",dst1);
    imshow("dst2",dst2);
    imshow("dst3",dst3);
    imshow("dst4",dst4);
    imshow("dst5",dst5);
    waitKey();
}
Mat drawHist(const std::array<int,256>& arr,const int &height){
    Mat hist(height,256,CV_8UC1,Scalar(0,0,0));
    int max = *std::max_element(arr.begin(),arr.end());
    double scale = (height - 30)/(double)max;
    for(int i = 0;i!=256;i++){
        int h = (int)arr[i]*scale;
        line(hist,{i,height-1},{i,height-1-h},Scalar(255,255,255));
    }
    return hist;
}
void otsu(const Mat &src,Mat &dst){
    auto arr = getArr(src);
    int n = src.rows*src.cols;
    long long frontSize =0 ,backSize=n;
    long long frontSum = 0,backSum=0;
    double max =0,now = 0;
    double maxCount=0;
    for(int i = 0;i!=256;i++){
        backSum += i*arr[i];
    }
    for(int i = 0;i!=255;i++){
        frontSize += arr[i];
        backSize -= arr[i];
        frontSum += i*arr[i];
        backSum -= i*arr[i];
        double w0 = frontSum/(double)frontSize;
        double w1 = backSum/(double)backSize;
        now = frontSize*backSize*(w0-w1)*(w0-w1);
        if(now>max){
            max = now;
            maxCount = i;
        }
    }
    std::cout << maxCount <<'\n';
    Mat lookUpTo(1,256,CV_8U);
    uchar *p = lookUpTo.ptr();
    for(int i = 0;i!=256;i++) i<=maxCount ? p[i] = 0 : p[i] = 255;
    LUT(src,lookUpTo,dst);
}
std::array<int ,256> getArr(const Mat &src){
    std::array<int,256> arr{};
    for(int y = 0;y!=src.rows;++y){
        for(int x = 0;x != src.cols;++x){
            arr[src.at<uchar>(y,x)]++;
        }
    }
    return arr;
}