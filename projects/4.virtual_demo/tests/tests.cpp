#include "filter/filter.hpp"
#include "pipe_line.hpp"
#include "gray_scale.hpp"
#include "gauss_blur.hpp"
#include <iostream>
#include <memory>
#include <opencv2/opencv.hpp>
using namespace cv;
int main()
{
  Mat src = imread("assets/1.jpg", IMREAD_COLOR);
  pipe_line p;
  p.add(std::make_unique<gray_scale>());
  p.add(std::make_unique<gauss_blur>());
  p.run(src);
  namedWindow("dst", WINDOW_AUTOSIZE);
  imshow("dst", src);
  waitKey(0);
  return 0;
}