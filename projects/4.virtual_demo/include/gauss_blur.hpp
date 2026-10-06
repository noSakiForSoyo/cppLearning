#pragma once

#include "filter/filter.hpp"

class gauss_blur : public filter
{
public:
  ~gauss_blur();
  void apply(cv::Mat &img) override;
  const char *name() const override;
};