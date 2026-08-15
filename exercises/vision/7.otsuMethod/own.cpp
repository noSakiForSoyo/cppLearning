#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
Mat drawHist(const std::array<int,256>& arr,const int &height);
std::array<int ,256> getArr(const Mat &src);
void otsu(const Mat &src,Mat &dst);
int main(){
    Mat src = imread("../a.jpg",IMREAD_GRAYSCALE);
    if(src.empty()){
        std::cerr << "文件未打开";
        return -1;
    }
    Mat dst(src.size(),src.type());   
    auto arr = getArr(src);
    Mat hist = drawHist(arr,300);
    otsu(src,dst);
    namedWindow("hist",WINDOW_AUTOSIZE);
    namedWindow("otsu",WINDOW_AUTOSIZE);
    imshow("hist",hist);
    imshow("otsu",dst);
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
    uchar maxCount=0;
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