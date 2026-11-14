#include <chrono>
#include <iostream>
#include <utility>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include "pipeline/pipeline.hpp"

class SyntheticSource : public Source
{
public:
  SyntheticSource(int n, int w, int h)
      : n_(n), w_(w), h_(h) {}

  bool read(Frame &frame) override
  {
    if (i_ >= n_)
      return false;

    frame.image.create(h_, w_, CV_8UC3);
    cv::randu(frame.image, cv::Scalar(0, 0, 0),
              cv::Scalar(255, 255, 255));
    frame.id = i_++;
    return true;
  }

  int produced() const { return i_; }

private:
  int n_;
  int w_;
  int h_;
  int i_ = 0;
};

class BlurStage : public Stage
{
public:
  bool process(Frame &frame) override
  {
    if (frame.image.empty())
      return false;

    cv::Mat out;
    cv::GaussianBlur(frame.image, out, cv::Size(9, 9), 0);
    frame.image = std::move(out);
    return true;
  }
};

class NullSink : public Sink
{
public:
  bool write(const Frame &) override
  {
    ++count;
    return true;
  }
  int count = 0;
};

static double now_ms()
{
  using namespace std::chrono;
  return duration<double, std::milli>(
             steady_clock::now().time_since_epoch())
      .count();
}

static void run_case(const char *name, int frames, std::size_t cap)
{
  SyntheticSource source(frames, 640, 480);
  BlurStage stage;
  NullSink sink;

  const double t0 = now_ms();

  if (cap == 0)
  {
    Frame frame;
    while (source.read(frame))
    {
      if (!stage.process(frame) || !sink.write(frame))
        break;
    }
  }
  else
  {
    Pipeline pipeline(source, stage, sink, cap);
    pipeline.run();
  }

  const double t1 = now_ms();
  const double ms = t1 - t0;
  const double fps = sink.count * 1000.0 / ms;

  std::cout << name
            << " produced=" << source.produced()
            << " processed=" << sink.count
            << " time=" << ms << " ms"
            << " fps=" << fps << "\n";
}

int main()
{
  const int frames = 300;

  run_case("single      ", frames, 0);
  run_case("pipe cap=64 ", frames, 64);
  run_case("pipe cap=3  ", frames, 3);
}