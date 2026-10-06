#include <iostream>
#include "filter/filter.hpp"
#include "gauss_blur.hpp"
using namespace cv;

gauss_blur::~gauss_blur()
{
}
void gauss_blur::apply(cv::Mat &img)
{
  GaussianBlur(img, img, Size(3, 3), 0);
}
const char *gauss_blur::name() const
{
  return "gauss_blur";
}