#include "../pipeline/sink.hpp"
#include <opencv2/videoio.hpp>
class VideoSink : public Sink
{
public:
    VideoSink(const std::string &path, double fps, cv::Size size);
    bool isOpened() const;
    bool write(const Frame &frame) override;
    double writeTime()const override;

private:
    cv::VideoWriter writer_;
    cv::Size size_;
    double t_write;
};