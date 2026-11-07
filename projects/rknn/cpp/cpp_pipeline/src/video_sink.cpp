#include "video/video_sink.hpp"
#include "postprocess/postprocess.hpp"
#include <cstdio>

VideoSink::VideoSink(const std::string &path, double fps, cv::Size size)
    : size_(size)
{
    // 板端走硬件编码：appsrc -> videoconvert -> NV12 -> mpph264enc。
    //
    // caps 里的 format=NV12 不能省：不写的话 videoconvert 会协商出一个
    // mpph264enc 内部 RGA 转不动的格式，运行时打 "failed to convert frame /
    // not negotiated"，现象是 writer.isOpened()==true、每帧 write 都失败、
    // 文件永远是 0 字节（OpenCV 只在 stderr 打一行 "Error pushing buffer"）。
    char caps[128];
    std::snprintf(caps, sizeof(caps),
                  "video/x-raw,format=NV12,width=%d,height=%d,framerate=%d/1",
                  size.width, size.height, (int)std::lround(fps));

    std::string pipeline =
        std::string("appsrc ! videoconvert ! videorate ! ") + caps +
        " ! mpph264enc ! h264parse ! mp4mux ! filesink location=" + path;

    writer_.open(pipeline, cv::CAP_GSTREAMER, 0, fps, size);
    if (!writer_.isOpened())
    {
        std::printf("!! VideoSink: GStreamer 管道打不开，退回软件编码（慢一个数量级）\n   %s\n",
                    pipeline.c_str());
        writer_.open(path, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, size);
        if (!writer_.isOpened())
            std::printf("!! VideoSink: 软件 mp4v 也打不开: %s\n", path.c_str());
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
    double n0 = now_ms();
    writer_.write(frame.image);
    t_write += now_ms() - n0;
    return true;
}

double VideoSink::writeTime() const
{
    return t_write;
}