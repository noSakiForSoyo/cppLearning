#pragma once

#include "frame.hpp"

class Source
{
public:
  virtual ~Source() = default;
  virtual bool read(Frame &frame) = 0;
  virtual double readTime() const = 0;
  virtual uint64_t produced() const = 0;
};