#include "../pipeline/source.hpp"
#include <opencv2/videoio.hpp>
class VideoFileSource : public Source{
public:
  explicit VideoFileSource(const std::string &path);
  bool isOpened() const;
  bool read(Frame &frame) override;
private:
  cv::VideoCapture cap_;
  uint64_t next_id_ = 0;
};