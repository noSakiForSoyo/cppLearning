#pragma once

#include "filter/filter.hpp"

class gray_scale : public filter
{
public:
  ~gray_scale();
  void apply(cv::Mat &img) override;
  const char *name() const override;
};