#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
float max(const std::initializer_list<float> &nums);
void BGRtoHSV(Mat &src,Mat &hsv);
void BGRtoHSV(Mat &src,Mat &hsv);
int main(){
    Mat src;
    src = imread("../a.jpg");
    Mat hsv = Mat::zeros(src.size(),CV_32FC3);

    BGRtoHSV(src,hsv);
    namedWindow("output",WINDOW_AUTOSIZE);
    imshow("output",hsv);
    waitKey();
}

void BGRtoHSV(Mat &src,Mat &hsv){
     for(int y = 0;y != src.rows;++y){
        for(int x = 0;x !=src.cols;++x){
            
            auto a = src.at<Vec3b>(y,x);
            float B = a[0]/255.0f;
            float G = a[1]/255.0f;
            float R = a[2]/255.0f;
            float mx = std::max({B,G,R});
            float mn = std::min({B,G,R});
            float d = mx - mn;
            float v = mx;
            
            //未进行v=0的检测
                float s = d/v;
            
                float h = 0.0f;
                        
            //未进行v=0的检测，且第一个公式错误
            if(v == B){
                h = saturate_cast<float>((240+60*((R-G)/(s*v))));
            }else if(value == G){
                h = saturate_cast<float>((120+60*((B-R)/(s*value))));
            }else if(value == R&&a[1]<a[0]){
                h = saturate_cast<float>((360+60*((G-B)/(s*value))));
            }else if(value == R&&a[1]<=a[0]){
                h = saturate_cast<float>((60*((G-B)/(s*value))));
            }
            hsv.at<Vec3f>(y,x) =Vec3f(h,s,v);
        }
     }
}