#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char **argv)
{
  Mat image = imread("assets/images/1.jpg",IMREAD_COLOR);
  if(image.empty())
  {
    std::cout << "Could not read the image: " << std::endl;
    return -1;
  }
  Mat gray;
  cvtColor(image, gray, COLOR_BGR2GRAY);
  namedWindow("src", WINDOW_AUTOSIZE);
  namedWindow("gray", WINDOW_AUTOSIZE);
  imshow("src", image);
  imshow("gray", gray);
  waitKey(0);
  return 0;
}