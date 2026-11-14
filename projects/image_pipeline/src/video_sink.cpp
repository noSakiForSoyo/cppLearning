#include "video/video_sink.hpp"

#include <cstdio>

VideoSink::VideoSink(const std::string &path, double fps, cv::Size size)
    : size_(size)
{
    writer_.open(path, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, size);
    if (!writer_.isOpened())
    {
        auto fallback = path.substr(0, path.rfind('.')) + "_fallback.avi";
        writer_.open(fallback, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                     fps, size);
        if (!writer_.isOpened())
            std::printf("VideoSink writer open failed\n");
    }
}

bool VideoSink::isOpened() const
{
    return writer_.isOpened();
}

bool VideoSink::write(const Frame &frame)
{
    if (frame.image.size() != size_)
        return false;

    writer_.write(frame.image);
    return true;
}
