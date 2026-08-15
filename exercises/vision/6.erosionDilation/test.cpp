#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

Mat a,erosion_dst,dilation_dst;

int erosion_size = 0;
int dilation_size = 0;
const int max_size = 21; 
const int size = 3;
const int element[size][size] = {{10,10,10},{10,10,10},{10,10,10}};
void erosion(const Mat &src,Mat &dst ,const int element[3][3]);
void dilation(const Mat &src,Mat &dst ,const int element[3][3]);
void erosionByAPI(int , void*);
void dilationByAPI(int , void*);
int main(){
    a = imread("../a.jpg",IMREAD_GRAYSCALE);
    if(a.empty()){
        std::cout << "文件名或类型错误\n";
        return -1; 
    }
    Mat dst1;
    erosion(a,dst1,element);
    Mat dst2;
    dilation(a,dst2,element);
    namedWindow("in",WINDOW_AUTOSIZE);
    namedWindow("out1",WINDOW_AUTOSIZE);
    namedWindow("out2",WINDOW_AUTOSIZE);
    namedWindow("erosion",WINDOW_AUTOSIZE);
    namedWindow("dilation",WINDOW_AUTOSIZE);
    createTrackbar("腐蚀滑动窗口","erosion",&erosion_size,max_size,erosionByAPI);
    createTrackbar("膨胀滑动窗口","dilation",&dilation_size,max_size,dilationByAPI);
    imshow("out1",dst1);
    imshow("out2",dst2);
    imshow("in",a);
    waitKey();
}

void erosion(const Mat &src,Mat &dst ,const int element[3][3]){
    uchar min = 255;
    if(dst.empty()){
        dst = Mat(src.size(),src.type());
    }
    for(int y = 1;y != src.rows-1;++y){
        for(int x = 1;x != src.cols-1;++x){
            for(int ey = 0;ey!=size;++ey){
                for(int ex = 0;ex !=size;ex++){
                    auto a = src.ptr<uchar>(y-1+ey);
                    uchar i = a[x-1+ex] - element[ey][ex];
                    if(i < min){
                        min = i;
                   }
                }
            }
            dst.at<uchar>(y,x) = min;
            min = 255;
        }
    }
}
void dilation(const Mat &src,Mat &dst ,const int element[3][3]){
    uchar max = 0;
    if(dst.empty()){
        dst = Mat(src.size(),src.type());
    }
    for(int y = 1;y != src.rows-1;++y){
        for(int x = 1;x != src.cols-1;++x){
            for(int ey = 0;ey!=size;++ey){
                for(int ex = 0;ex !=size;ex++){
                    auto a = src.ptr<uchar>(y-1+ey);
                    uchar i = a[x-1+ex] + element[ey][ex];
                    if(i > max){
                        max = i;
                   }
                }
            }
            dst.at<uchar>(y,x) = max;
            max = 0;
        }
    }
}
void erosionByAPI(int , void*){
    Mat e= getStructuringElement(MORPH_RECT,
        Size(2*erosion_size+1,2*erosion_size+1),
        Point(erosion_size,erosion_size));
    erode (a,erosion_dst,e); 
    imshow("erosion",erosion_dst);
}
void dilationByAPI(int , void*){
    Mat e = getStructuringElement(MORPH_RECT,
        Size(2*dilation_size+1,2*dilation_size+1),
        Point(dilation_size,dilation_size));
    dilate(a,dilation_dst,e); 
    imshow("dilation",dilation_dst);
}