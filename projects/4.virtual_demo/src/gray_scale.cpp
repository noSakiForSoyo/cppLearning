#include <iostream>
#include "filter/filter.hpp"
#include "gray_scale.hpp"
using namespace cv;

gray_scale::~gray_scale()
{
}
void gray_scale::apply(cv::Mat &img)
{
  cvtColor(img, img, COLOR_BGR2GRAY);
}
const char *gray_scale::name() const
{
  return "gray_scale";
}