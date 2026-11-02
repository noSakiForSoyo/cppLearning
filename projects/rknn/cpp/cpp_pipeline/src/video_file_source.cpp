#include "video/video_file_source.hpp"

#include <opencv2/videoio.hpp>

VideoFileSource::VideoFileSource(const std::string &path)
    : cap_(path, cv::CAP_FFMPEG) {}

bool VideoFileSource::isOpened() const
{
    return cap_.isOpened();
}

bool VideoFileSource::read(Frame &frame)
{
    if (!cap_.read(frame.image))
        return false;

    frame.id = next_id_++;
    frame.timestamp = cap_.get(cv::CAP_PROP_POS_MSEC) / 1000.0;
    return true;
}
