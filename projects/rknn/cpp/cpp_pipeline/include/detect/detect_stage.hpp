#pragma once

#include "pipeline/stage.hpp"
#include "rknn/rknn_api.h"
#include <string>
#include <opencv2/imgproc.hpp>
class DetectStage : public Stage
{
public:
  DetectStage(const char *model_path,
      float conf_thres = 0.25f,
      float iou_thres = 0.5f);
  ~DetectStage();
  bool isReady() const;
  bool process(Frame &frame) override;
  void printStats() const override;
  int frames()const override;
private:
  bool init_(const char *path);
  void release_();
  rknn_context ctx_ = 0;
  rknn_input_output_num io_{};
  rknn_tensor_attr in_attr_;
  rknn_tensor_attr out_attrs_[2]{};
  rknn_output *outs_=nullptr;
  int MW_ = 0, MH_ = 0;
  int pad_w_ = 0, pad_h_ = 0;
  float conf_ = 0.25f, iou_ = 0.5f,scale_;
  double elapsed_=0 ;
  double t_letter_ = 0, t_input_ = 0, t_run_ = 0,
         t_get_ = 0, t_decode_ = 0, t_draw_ = 0;
  int frames_ = 0;
  cv::Mat rgb_;
  bool ready_ = false , outs_got_ = false;
};