#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
Mat drawHist(const std::array<int,256> &arr,int height);
void equalize(Mat &src,Mat &dst);
int main(){
    //记得初始化，这里由于未初始化造成
    std::array<int,256> arr{},brr{},crr{};
    Mat src;
    src = imread("../a.jpg",IMREAD_COLOR);
    if (src.empty()) {
        std::cout << "错误：无法读取图片，请检查路径和文件名！" << std::endl;
        return -1; 
    }
    cvtColor(src,src,COLOR_BGR2GRAY);
    Mat eqz;
    Mat owneqz = Mat::zeros(src.size(),src.type());
    equalizeHist(src,eqz);
    equalize(src,owneqz);
    for(int y = 0;y < src.rows;++y){
        for(int x = 0;x <src.cols;++x){
            ++arr[src.at<uchar>(y,x)];
            ++brr[eqz.at<uchar>(y,x)];
            ++crr[owneqz.at<uchar>(y,x)];
        }
    } 

    Mat img = drawHist(arr,300);  
    eqz = drawHist(brr,300);
    owneqz = drawHist(crr,300);
    // int channels[] = {0};
    // int histSize[] = {256};
    // float hranges[] = {0,256};
    // const float *range[] = {hranges};
    // Mat result;
    // calcHist(&src,1,channels,Mat(),result,1,histSize,range);
    // std::array<int, 256> hist_arr; 
    // for(int i = 0; i < 256; i++) {
    //     hist_arr[i] = (int)result.at<float>(i); 
    // }
    // Mat hist_img = drawHist(hist_arr,300);
    // namedWindow("hist_img",WINDOW_AUTOSIZE);
    // imshow("hist_img",hist_img);

    namedWindow("src",WINDOW_AUTOSIZE);
    namedWindow("hist",WINDOW_AUTOSIZE);
    namedWindow("histd",WINDOW_AUTOSIZE);
    namedWindow("ownhist",WINDOW_AUTOSIZE);

    imshow("src",src);
    imshow("hist",img);
    imshow("histd",eqz);
    imshow("ownhist",owneqz);

    waitKey();
}
//绘制直方图
Mat drawHist(const std::array<int,256> &arr,int height){
    Mat hist(height,256,CV_8UC3,Scalar(0,0,0));
    int maxCount = *std::max_element(arr.begin(),arr.end());
    double scale = (height-30)/(double)maxCount;
    for(int i = 0 ; i!=256 ; i++){
        int h = (int)arr[i]*scale;
        line(hist,{i,height-1},{i,height-1-h},Scalar(255,255,255));
    }
    return hist;
}
//均衡化
void equalize(Mat &src,Mat &dst){
    auto m = src.rows*src.cols;
    std::array<int ,256> arr{},brr{};
    for(int y = 0;y!=src.rows;++y){
        for(int x = 0;x!=src.cols;++x){
            arr[src.at<uchar>(y,x)]++;
        }
    }
    brr[0] = arr[0];
    for(int i = 1;i!=256;++i){
        brr[i] = brr[i-1] + arr[i];
    }
    int cdf_min = 0;
    for(int i = 0;i!=256;++i){
        if(brr[i]>0){
            cdf_min = brr[i];
            break;
        }
    }
    float div = m - cdf_min;
    if(div<1e-6){
        div = 1;
    }
    std::array<uchar,256> lut{};
    for(int i = 0;i!=256;++i){
        if(brr[i]>0){
            lut[i] = saturate_cast<uchar>((brr[i] - cdf_min)*255.0f / div); 
        }else{
            lut[i]=0;
        }
    }
    for(int y = 0;y!=dst.rows;++y){
        for(int x = 0 ;x!=dst.cols;++x){
            dst.at<uchar>(y,x) = lut[src.at<uchar>(y,x)];
        }
    }
}