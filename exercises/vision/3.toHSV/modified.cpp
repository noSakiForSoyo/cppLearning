#include <iostream>

#include <opencv2/opencv.hpp>
using namespace cv;
float max(const std::initializer_list<float> &nums);
void BGR2HSV(Mat &src,Mat &hsv);
void HSV2BGR(Mat &hsv,Mat &bgr);
int main(){
    Mat src;
    src = imread("../test.jpeg");
    Mat hsv = Mat::zeros(src.size(),CV_32FC3);

    BGR2HSV(src,hsv);
    namedWindow("input",WINDOW_AUTOSIZE);
    namedWindow("hsvput",WINDOW_AUTOSIZE);
    namedWindow("bgrput",WINDOW_AUTOSIZE);
    Mat news = Mat::zeros(src.size(),CV_32FC3);
    HSV2BGR(hsv,news);
    // cv::cvtColor(src,hsv,COLOR_BGR2HSV);
    // cv::cvtColor(hsv,news,COLOR_HSV2BGR);
    
    imshow("input",src);
    imshow("hsvput",hsv);
    imshow("bgrput",news);
    waitKey();
}

void BGR2HSV(Mat &src,Mat &hsv){
    for(int y = 0;y != src.rows;++y){
        for(int x = 0;x !=src.cols;++x){
            
            auto &a = src.at<Vec3b>(y,x);
            float B = a[0]/255.0f;
            float G = a[1]/255.0f;
            float R = a[2]/255.0f;
            float mx = std::max({B,G,R});
            float mn = std::min({B,G,R});
            float d = mx - mn;
            float v = mx;
            float s;
            if(v > 1e-5f)
                s = d/v;
            else
                s = 0.0f;
            float h = 0.0f;
            if(d!=0){
                if(v > 1e-5f){            
                    if(v == B){
                        h = saturate_cast<float>((240+60*((R-G)/(d))));
                    }else if(v == G){
                        h = saturate_cast<float>((120+60*((B-R)/(d))));
                    }else if(v == R&&a[1]<a[0]){
                        h = saturate_cast<float>((360+60*((G-B)/(d))));
                    }else if(v == R&&a[1]<=a[0]){
                        h = saturate_cast<float>((60*((G-B)/(d))));
                    }
                    if(h<0){
                        h +=360.0f;
                    }
                }
            }
            hsv.at<Vec3f>(y,x) = Vec3f(h,s,v);
        }
    }
}
void HSV2BGR(Mat &hsv,Mat &bgr){
    for(int y = 0;y!=hsv.rows;++y){
        for(int x = 0;x!=hsv.cols;++x){
            Vec3f &a = hsv.at<Vec3f>(y,x);
            Vec3f &b = bgr.at<Vec3f>(y,x);
            float H = a[0];
            float S = a[1];
            float V = a[2];
            int h60f = std::floor(H/60.0f);//取最接近的颜色
            auto hi = h60f % 6;//保证落在0-5之间
            float f = H/60.0f - h60f;//走过的百分比
            float P = V*(1-S); //最小值
            float Q = V*(1-f*S);//减少量
            float T = V*(1-(1-f)*S);//增加量
             if(y ==10 && x ==10){
                std::cout <<' '<< H <<' '<< S <<' '<< V <<' '<<h60f<<' '<<hi<<' '<<f<<' '<<P<<' '<<Q<<' '<<T<<'\n';
             }
            if(hi == 0){
                b[0] = P;
                b[1] = T;
                b[2] = V; 
            }else if(hi == 1){
                b[0] = P;
                b[1] = V;
                b[2] = Q; 
            }else if(hi == 2){
                b[0] = T;
                b[1] = V;
                b[2] = P; 
            }else if(hi == 3){
                b[0] = V;
                b[1] = Q;
                b[2] = P; 
            }else if(hi == 4){
                b[0] = V;
                b[1] = P;
                b[2] = T; 
            }else if(hi == 5){
                b[0] = Q;
                b[1] = P;
                b[2] = V; 
            }
        }
    }
}