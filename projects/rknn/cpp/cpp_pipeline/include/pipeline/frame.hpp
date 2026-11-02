#pragma once

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <cstdint>
struct Frame{
  cv::Mat image;
  uint64_t id;
  double timestamp=0.0;
};