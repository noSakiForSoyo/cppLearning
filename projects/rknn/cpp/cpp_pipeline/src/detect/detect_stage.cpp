#include "detect/detect_stage.hpp"
#include "postprocess/postprocess.hpp"
#include <ctime>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

DetectStage::DetectStage(const char *model_path,
                         float conf_thres,
                         float iou_thres)
{
  init_(model_path);
  conf_ = conf_thres;
  iou_ = iou_thres;
}
bool DetectStage::isReady() const
{
  return ready_;
}
DetectStage::~DetectStage()
{
  release_();
}
int DetectStage::frames()const{
  return frames_;
}
void DetectStage::release_()
{
  if (outs_got_ && ctx_)
    rknn_outputs_release(ctx_, io_.n_output, outs_);
  if (outs_)
    free(outs_);
  if (ctx_)
    rknn_destroy(ctx_);
  ctx_ = 0;
  outs_ = nullptr;
}
bool DetectStage::process(Frame &frame)
{
  double p1 = now_ms();
  rgb_ = letterbox(frame.image, MW_, MH_, &scale_, &pad_w_, &pad_h_);
  t_letter_ += now_ms() - p1;
  if (rgb_.empty() || rgb_.cols != MW_ || rgb_.rows != MH_)
  {
    printf("!! letterbox 返回空或尺寸不对\n");
    release_();
    return false;
  }
  if (!rgb_.isContinuous())
    rgb_ = rgb_.clone();
  rknn_input in;
  memset(&in, 0, sizeof(in));
  in.index = 0;
  in.type = RKNN_TENSOR_UINT8;
  in.fmt = RKNN_TENSOR_NHWC;
  in.size = (uint32_t)(MW_ * MH_ * 3);
  in.buf = rgb_.data;
  in.pass_through = 0;
  double p2 = now_ms();
  int ret = rknn_inputs_set(ctx_, 1, &in);
  t_input_ += now_ms() - p2;
  if (ret < 0)
  {
    printf("inputs_set failed %d\n", ret);
    release_();
    return false;
  }
  double t2 = now_ms();
  ret = rknn_run(ctx_, NULL);
  double t3 = now_ms();
  t_run_ += t3 - t2;
  if (ret < 0)
  {
    printf("run failed\n");
    release_();
    return false;
  }

  ret = rknn_outputs_get(ctx_, io_.n_output, outs_, NULL);
  if (ret < 0)
  {
    printf("outputs_get failed %d\n", ret);
    release_();
    return false;
  }
  outs_got_ = true;
  double t4 = now_ms();
  t_get_ += t4 - t3;
  // printf("infer %.2f ms,  outputs_get %.2f ms\n", t3 - t2, t4 - t3);
  const int8_t *coords = (const int8_t *)outs_[0].buf;
  const int8_t *scores = (const int8_t *)outs_[1].buf;
  const int NA = out_attrs_[0].dims[2], NC = out_attrs_[1].dims[1];

  double t5 = now_ms();
  std::vector<Detection> dets =
      decode_int8(coords, scores, NA, NC, out_attrs_[0].zp, out_attrs_[0].scale, out_attrs_[1].zp, out_attrs_[1].scale, conf_, iou_);
  double t6 = now_ms();
  t_decode_ += t6 - t5;
  double t7 = now_ms();
  for (auto det : dets)
  {
    char text[64];
    snprintf(text, sizeof(text), "cls=%d %.2f", det.cls, det.score);

    float x1 = (det.box.x1 - pad_w_) / scale_;
    float x2 = (det.box.x2 - pad_w_) / scale_;
    float y1 = (det.box.y1 - pad_h_) / scale_;
    float y2 = (det.box.y2 - pad_h_) / scale_;
    cv::rectangle(frame.image, cv::Rect(x1, y1, x2 - x1, y2 - y1), cv::Scalar(0, 255, 0), 2);
    putText(frame.image, text, cv::Point((int)x1, (int)y1 - 4), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 0, 255), 2);
  }
  t_draw_ += now_ms() - t7;
  rknn_outputs_release(ctx_, io_.n_output, outs_);
  outs_got_ = false;
  frames_++;
  elapsed_ += now_ms() - p1;
  return true;
}
void DetectStage::printStats() const
{
  if (frames_ == 0)
  {
    printf("no frames processed\n");
    return;
  }
  printf("frames=%d, avg=%.2f ms, fps=%.2f, "
         "t_decode=%f, t_get_out=%f, t_run=%f, t_letter=%f, t_input=%f, t_draw=%f\n",
         frames_, elapsed_ / frames_,
         frames_ * 1000.0 / elapsed_,
         t_decode_ / frames_, t_get_ / frames_, t_run_ / frames_, t_letter_ / frames_, t_input_ / frames_, t_draw_ / frames_);
}
bool DetectStage::init_(const char *path)
{
  FILE *f = fopen(path, "rb");
  if (f == nullptr)
  {
    printf("path错误,detectStage init_失败");
    release_();
    return false;
  }
  fseek(f, 0, SEEK_END);
  long msz = ftell(f);
  fseek(f, 0, SEEK_SET);
  void *model = malloc(msz);
  if (fread(model, 1, msz, f) != (size_t)msz)
  {
    printf("read model failed\n");
    fclose(f);
    free(model);
    return false;
  }
  fclose(f);
  ctx_ = 0;
  int ret = rknn_init(&ctx_, model, (int)msz, 0, NULL);
  free(model);
  if (ret < 0)
  {
    printf("rknn_init failed %d\n", ret);
    release_();
    return false;
  }
  memset(&io_, 0, sizeof(io_));

  memset(&in_attr_, 0, sizeof(in_attr_));

  rknn_query(ctx_, RKNN_QUERY_IN_OUT_NUM, &io_, sizeof(io_));
  in_attr_.index = 0;
  rknn_query(ctx_, RKNN_QUERY_INPUT_ATTR, &in_attr_, sizeof(in_attr_));

  MH_ = (int)in_attr_.dims[1];
  MW_ = (int)in_attr_.dims[2];
  printf("model loaded, input %dx%d (W x H), outputs=%u\n", MW_, MH_, io_.n_output);
  if (io_.n_output != 2)
  {
    printf("!! 这个程序假设 2 个输出(coords/scores),实际 %u 个\n", io_.n_output);
    release_();
    return false;
  }
  memset(out_attrs_, 0, sizeof(out_attrs_));

  for (uint32_t i = 0; i < io_.n_output; ++i)
  {
    out_attrs_[i].index = i;
    ret = rknn_query(ctx_, RKNN_QUERY_OUTPUT_ATTR,
                     &out_attrs_[i], sizeof(rknn_tensor_attr));
    if (ret < 0)
    {
      printf("query output %u failed %d\n", i, ret);
      release_();
      return false;
    }
    printf("out %u name=%s dims=[%u,%u,%u] zp=%d scale=%f size=%u\n",
           i, out_attrs_[i].name,
           out_attrs_[i].dims[0],
           out_attrs_[i].dims[1],
           out_attrs_[i].dims[2],
           out_attrs_[i].zp,
           out_attrs_[i].scale,
           out_attrs_[i].size);
  }
  outs_ = (rknn_output *)calloc(io_.n_output, sizeof(rknn_output));
  if (!outs_)
  {
    printf("calloc outs failed\n");
    release_();
    return false;
  }
  for (uint32_t i = 0; i < io_.n_output; ++i)
  {
    outs_[i].index = i;
    outs_[i].want_float = 0;
  }
  ready_ = true;
  return true;
}