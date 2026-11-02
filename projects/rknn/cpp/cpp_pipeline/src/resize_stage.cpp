#include "video/resize_stage.hpp"
#include <opencv2/imgproc.hpp>

ResizeStage::ResizeStage(cv::Size out_size) : out_size_(out_size) {}

bool ResizeStage::process(Frame &frame)
{
  if (frame.image.size() == out_size_)
  {
    return true;
  }
  cv::Mat dst;
  cv::resize(frame.image, dst, out_size_);
  frame.image = std::move(dst);
  return true;
}