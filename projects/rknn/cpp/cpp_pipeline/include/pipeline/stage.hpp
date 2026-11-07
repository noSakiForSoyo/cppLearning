#pragma once

#include "frame.hpp"
class Stage
{
public:
  virtual ~Stage() = default;
  virtual bool process(Frame &frame) = 0;
  virtual void printStats() const = 0;
  virtual int frames() const = 0;
};