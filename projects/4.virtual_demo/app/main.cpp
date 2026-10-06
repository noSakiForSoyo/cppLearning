#include <iostream>
#include "filter/filter.hpp"
#include <opencv2/opencv.hpp>
#include "pipe_line.hpp"
#include "gray_scale.hpp"
#include "gauss_blur.hpp"
#include <filesystem>
#include <thread>
#include <chrono>
#include <atomic>

thread_local cv::Mat scratch;
std::atomic<int> next_task{0};
void createTestImage()
{
  std::filesystem::create_directory("test_images");
  for (int i = 0; i < 100; i++)
  {
    cv::Mat img(1000, 1000, CV_8UC3, cv::Scalar(i * 25, i * 25, i * 25));
    cv::circle(img, cv::Point(50, 50), 10, cv::Scalar(255 - i * 25, 255 - i * 25, 255 - i * 25), -1);
    std::string filename = "test_images/test_image_" + std::to_string(i) + ".png";
    cv::imwrite(filename, img);
  }
}
std::atomic<long long> us_read{0}, us_proc{0}, us_write{0};
void processOne(const std::string& path){
    using clk = std::chrono::steady_clock;

    auto t0 = clk::now();
    cv::Mat img = cv::imread(path);
    auto t1 = clk::now();
    if (img.empty()) return;

    pipe_line pl;
    pl.add(std::make_unique<gray_scale>());
    pl.add(std::make_unique<gauss_blur>());
    pl.run(img);
    auto t2 = clk::now();

    std::string out = "output_images/" +
                      std::filesystem::path(path).filename().string();
    cv::imwrite(out, img);
    auto t3 = clk::now();

    us_read  += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    us_proc  += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    us_write += std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();
}

double runsingle(const std::vector<std::string> &file)
{us_read = 0;  us_proc = 0;  us_write = 0;
  auto t = std::chrono::steady_clock::now();
  for (auto &f : file)
  {
    processOne(f);
  }
  auto t2 = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t).count();
}
double runmulti(const std::vector<std::string> &file, int nthreads)
{us_read = 0;  us_proc = 0;  us_write = 0;
  auto t = std::chrono::steady_clock::now();
  next_task = 0;
  std::vector<std::thread> threads;
  for (int i = 0; i < nthreads; i++)
  {
    threads.emplace_back([&file]()
                         {
      int total = file.size();
      int j = 0;
      while((j=next_task.fetch_add(1)) <total){
        processOne(file[j]);
      } });
  }
  for (auto &t : threads)
  {
    t.join();
  }
  auto t2 = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t).count();
}
void report(const std::string& tag){
    std::cout << tag
              << "   读 "   << us_read.load()  / 1000 << " ms"
              << "   处理 " << us_proc.load()  / 1000 << " ms"
              << "   写 "   << us_write.load() / 1000 << " ms\n";
}
int main()
{
  createTestImage();

  std::vector<std::string> files;
  for (int i = 0; i < 100; i++)
  {
    std::string filename = "test_images/test_image_" + std::to_string(i) + ".png";
    files.push_back(filename);
  }
  std::cout << "图片：" << files.size() << std::endl;

  double singleTime = runsingle(files);
  std::cout << "单线程处理时间: " << singleTime << " ms"
            << std::endl;report("        ");
  for (int i = 1; i <= 12; i++)
  {
    double multiTime = runmulti(files, i);
    std::cout << "多线程处理时间(" << i << "线程): " << multiTime << " ms"
              << std::endl;report("        ");
  }
}