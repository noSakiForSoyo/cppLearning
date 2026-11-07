#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "pipeline/pipeline.hpp"
#include "video/resize_stage.hpp"
#include "video/video_file_source.hpp"
#include "video/video_sink.hpp"
#include "video/camera_source.hpp"
#include "detect/detect_stage.hpp"

int main(int argc, char **argv)
{
    // 用法：./tests [运行秒数]   默认 15 秒
    int seconds = (argc > 1) ? atoi(argv[1]) : 15;
    if (seconds <= 0)
        seconds = 15;

    CameraSource src("/dev/video10");
    DetectStage  st("yolov8n_int8.rknn", 0.25f, 0.5f);
    if (!src.isOpened()) { printf("src no open\n"); return 1; }
    if (!st.isReady())   { printf("st not ready\n"); return 1; }

    VideoSink sink("out_cam_pipeline.mp4", 10, cv::Size(1280, 720));
    if (!sink.isOpened()) { printf("sink no open\n"); return 1; }

    Pipeline p(src, st, sink, 3);
    std::printf("=== running for %d seconds ===\n", seconds);

    // 定时器：到点就让 pipeline 停下来，这样 run() 会返回、printStats() 才会执行
    std::thread stopper([&p, seconds]()
                        {
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        p.stop(); });

    p.run();
    stopper.join();
    p.printStats();
    return 0;
}
