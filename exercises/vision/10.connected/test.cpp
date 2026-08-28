#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(){
    Mat src;
    src = imread("../a.jpg",IMREAD_GRAYSCALE);
    threshold(src,src,0,255,THRESH_BINARY|THRESH_OTSU);
    Mat labels,stats,centroids;
    int n = connectedComponentsWithStats(src,labels,stats,centroids);

    Mat color;
    cvtColor(src,color,COLOR_GRAY2BGR);
    int count=0;
    for(int i = 1;i<n;i++){
        int area = stats.at<int>(i,CC_STAT_AREA);
        if(area <= 100)continue;
        int x = stats.at<int>(i,CC_STAT_LEFT);
        int y = stats.at<int>(i,CC_STAT_TOP);
        int h = stats.at<int>(i,CC_STAT_HEIGHT);
        int w = stats.at<int>(i,CC_STAT_WIDTH);
        rectangle(color,Rect(x,y,w,h),Scalar(0,0,255));
        count++;
    }
    imshow("result",color);
    waitKey();
}