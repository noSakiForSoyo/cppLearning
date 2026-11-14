#include <iostream>
#include <string>

#include "pipeline/pipeline.hpp"
#include "video/resize_stage.hpp"
#include "video/video_file_source.hpp"
#include "video/video_sink.hpp"

int main(int argc, char **argv)
{
    const std::string path = argc > 1 ? argv[1] : "test_video_mp4v.mp4";

    VideoFileSource source(path);
    if (!source.isOpened())
    {
        std::cerr << "video source open failed: " << path << "\n";
        return 1;
    }

    ResizeStage stage(cv::Size(640, 360));
    VideoSink sink("pipeline_out.mp4", 30.0, cv::Size(640, 360));
    if (!sink.isOpened())
    {
        std::cerr << "video sink open failed\n";
        return 1;
    }

    Pipeline pipeline(source, stage, sink, 64);
    const bool ok = pipeline.run();

    std::cout << "ok=" << ok << "\n";
    return ok ? 0 : 1;
}
