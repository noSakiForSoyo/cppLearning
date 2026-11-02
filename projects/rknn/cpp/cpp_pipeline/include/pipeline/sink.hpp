#pragma once

#include "frame.hpp"
class Sink{
public:
  virtual ~Sink() = default;
  virtual bool write(const Frame &frame) = 0;
};