#pragma once
#include <opencv2/opencv.hpp>
class filter{
public:
  virtual ~filter() = default;
  virtual void apply(cv::Mat &img) = 0;
  virtual const char *name() const = 0;
};