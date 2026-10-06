#include <iostream>
#include <memory>
#include "filter/filter.hpp"
#include "pipe_line.hpp"

void pipe_line::add(std::unique_ptr<filter> f)
{
  types.push_back(std::move(f));
}
void pipe_line::run(cv::Mat &img)
{
  for (auto &a : types)
  {
    a->apply(img);
  }
}
size_t pipe_line::size() const
{
  return types.size();
}