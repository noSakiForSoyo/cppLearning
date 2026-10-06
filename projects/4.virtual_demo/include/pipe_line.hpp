#pragma once

#include <memory>
#include <vector>
#include "filter/filter.hpp"

class pipe_line
{
public:
  void add(std::unique_ptr<filter> f);
  size_t size() const;
  void run(cv::Mat &img);

private:
  std::vector<std::unique_ptr<filter>> types;
};
