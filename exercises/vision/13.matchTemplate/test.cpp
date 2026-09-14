#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
Mat src,src_gray,result;
Mat temp;

const std::string src_name = "src";
const std::string result_name = "result";
int match_type = 0; 
const int max_type = 1;
void matchMethod(int ,void *);
int main(){
    src = imread("../d.png",IMREAD_COLOR);
    temp = imread("../e.png",IMREAD_GRAYSCALE);
    namedWindow(src_name,IMREAD_COLOR);
    namedWindow(result_name,IMREAD_COLOR);
    createTrackbar("src",result_name,&match_type,max_type,matchMethod);
    matchMethod( 0, 0 );
    waitKey();
}
void matchMethod(int ,void *){
    Mat src_display ;
    src.copyTo(src_display);
    int result_col = src.cols - temp.cols + 1;
    int result_row = src.rows - temp.rows + 1;
    result.create(result_row,result_col,CV_32FC1);
    int method;
    cvtColor(src,src_gray,COLOR_BGR2GRAY);
    if(match_type == 0) method = TM_CCORR;
        else method = TM_CCORR_NORMED;
    matchTemplate(src_gray,temp,result,method);
    normalize(result,result,0,1,NORM_MINMAX,-1,Mat());
    double min_val,max_val;
    Point min_loc,max_loc,loc;
    minMaxLoc(result,&min_val,&max_val,&min_loc,&max_loc,Mat());
    loc = max_loc;
    rectangle(src_display,loc,Point(loc.x+temp.cols,loc.y+temp.rows),Scalar::all(0),2);
    rectangle(result,loc,Point(loc.x,loc.y),Scalar::all(0),2);
    std::cout << "min=" << min_val << " max=" << max_val << std::endl;
    imshow(src_name,src_display);
    imshow(result_name,result);
}