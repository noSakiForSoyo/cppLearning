#pragma once

#include "frame.hpp"
class Stage{
public:
  virtual ~Stage()=default;
  virtual bool process(Frame &frame) = 0;
};