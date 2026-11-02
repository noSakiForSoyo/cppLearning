# C++ Multithreaded Image Pipeline

A small reusable image-processing pipeline built with C++17 and OpenCV.
It separates data acquisition, processing, and output into Source / Stage / Sink,
and overlaps acquisition with processing through a bounded queue and one producer thread.

## Architecture

    VideoFileSource
          |
          v
    BoundedQueue<Frame>   capacity = 3 or 64
          |
          v
    Pipeline worker: Stage -> Sink
          |
          v
    ResizeStage -> VideoSink

The producer thread runs Source::read and pushes Frame objects into a bounded queue.
The consumer thread pops frames, runs Stage::process, then Sink::write.
When the queue is full, the oldest frame is dropped so latency stays bounded.

## Interfaces

    struct Frame {
        cv::Mat image;      // BGR
        uint64_t id;
        double timestamp;
    };

    class Source { virtual bool read(Frame&) = 0; };
    class Stage  { virtual bool process(Frame&) = 0; };
    class Sink   { virtual bool write(const Frame&) = 0; };

Concrete implementations:

- VideoFileSource: OpenCV VideoCapture with the FFMPEG backend
- ResizeStage: resize to 640x360
- VideoSink: OpenCV VideoWriter using mp4v
- NullSink: benchmark-only sink that discards frames

## Build

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build -j

Release mode matters for benchmark numbers.

## Run the video demo

    ./build/tests/tests /path/to/video.mp4

The output is pipeline_out.mp4 in the current directory.
The video source is opened through CAP_FFMPEG, so a file path is not parsed as a GStreamer pipeline.

## Benchmark

    ./build/tests/benchmark

Synthetic workload: 300 frames, 640x480 BGR, 9x9 GaussianBlur, NullSink.
Representative Release run on the WSL development machine:

    single        767.462 ms   390.90 FPS
    pipe cap=64   560.418 ms   535.32 FPS
    pipe cap=3    548.224 ms   547.22 FPS

All three cases processed 300 frames. The pipeline improved throughput by about 1.37-1.40x.
The cap=3 run did not drop frames in this workload because production and consumption
were roughly balanced; it is a queue-bound test, not a drop test.

Numbers vary by a few percent between runs. Run the benchmark several times before
quoting a final figure.

## Design notes

- Frame is moved into the queue; no shared mutable image between threads.
- The consumer owns the frame while running Stage and Sink.
- Stop wakes the consumer; it drains queued frames before returning false.
- Queue capacity 64 is used for lossless verification; capacity 3 is the real-time mode.
- The benchmark excludes disk I/O by using NullSink; the video demo measures the full path.

## Limitations and next work

- One worker thread, not a thread pool.
- No camera Source yet; VideoFileSource is the current input.
- No dropped-frame counter or per-stage metrics yet.
- Next: add a camera source, queue drop counters, and per-stage timing.

## Layout

    include/pipeline/   interfaces, bounded queue, pipeline declaration
    src/                pipeline and concrete video adapters
    tests/              video demo and synthetic benchmark
