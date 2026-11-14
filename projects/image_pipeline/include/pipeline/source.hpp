#pragma once

#include "frame.hpp"

class Source{
public:
  virtual ~Source() = default;
  virtual bool read(Frame &frame) = 0;
};