#pragma once
#include "pipeline/source.hpp"
#include <opencv2/videoio.hpp>

class CameraSource:public Source{
public:
  explicit CameraSource(const std::string &path);
  bool isOpened() const;
  bool read(Frame &frame) override;
  double readTime()const override;
  uint64_t produced() const override;
private:
  cv::VideoCapture cap_;
  uint64_t next_id_ = 0;
  cv::Mat raw_;
  double t_read;
};