#include "video/camera_source.hpp"
#include <chrono>
#include "postprocess/postprocess.hpp"
using namespace cv;
CameraSource::CameraSource(const std::string &path) : cap_(path, cv::CAP_V4L2)
{
  cap_.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
  cap_.set(CAP_PROP_FRAME_WIDTH, 1280);
  cap_.set(CAP_PROP_FRAME_HEIGHT, 720);
  cap_.set(CAP_PROP_FPS, 10);
  printf("camera %dx%d fps=%.2f\n",
         (int)cap_.get(CAP_PROP_FRAME_WIDTH),
         (int)cap_.get(CAP_PROP_FRAME_HEIGHT),
         cap_.get(CAP_PROP_FPS));
}
bool CameraSource::isOpened() const
{
  return cap_.isOpened() ? true : false;
}
bool CameraSource::read(Frame &frame)
{
  auto now = std::chrono::steady_clock::now();
  double t0 = now_ms();
  if (!cap_.read(raw_))
  {
    return false;
  }
  frame.timestamp = std::chrono::duration<double>(now.time_since_epoch()).count();
  frame.image = raw_.clone();
  frame.id = next_id_++;
  t_read+=now_ms() - t0;
  return true;
}
double CameraSource::readTime()const{
  return t_read;
}
uint64_t CameraSource::produced() const { return next_id_; }
