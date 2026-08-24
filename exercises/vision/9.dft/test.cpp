#include <iostream>
#include <opencv2/opencv.hpp>
using namespace cv;
int main(){
    Mat src;
    src = imread("../a.jpg",IMREAD_GRAYSCALE);
    Mat padding;
    int m = getOptimalDFTSize(src.rows);
    int n = getOptimalDFTSize(src.cols);
    copyMakeBorder(src,padding,0,m - src.rows,0,src.cols,BORDER_DEFAULT,Scalar(0,0,0));
    Mat planes[] = {Mat_<float>(padding),Mat::zeros(padding.size(),CV_32F)};
    Mat complexI;
    merge(planes,2,complexI);
    dft(complexI,complexI);

    split(complexI,planes);
    Mat magI;
    magnitude(planes[0],planes[1],magI);
    magI += Scalar::all(1);
    log(magI,magI);
    magI = magI(Rect(0, 0, magI.cols & -2, magI.rows & -2));
    int x = magI.cols/2;
    int y = magI.rows/2;
    Mat q0(magI,Rect(0,0,x,y));
    Mat q1(magI,Rect(x,0,x,y));
    Mat q2(magI,Rect(0,y,x,y));
    Mat q3(magI,Rect(x,y,x,y));

    Mat tmp;
    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
    
    normalize(magI, magI, 0, 1, NORM_MINMAX);
    namedWindow("img");
    imshow("img",magI);
    waitKey();
}