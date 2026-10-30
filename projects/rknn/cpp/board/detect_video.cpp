// detect_board.cpp —— 板端 YOLOv8n INT8 检测
//
// 编译（板子上）:
// g++ -O2 -std=c++17 -o detect_video detect_video.cpp postprocess.cpp \
//   -I. -I/usr/include/opencv4 -L/usr/lib -lrknnrt \
//   -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio
//
// 用法:
//  ./detect_video yolov8n_int8.rknn test_video_mp4v.mp4

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <ctime>
#include <vector>
#include <algorithm>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include "rknn_api.h"
#include "postprocess.hpp"

using namespace cv;

static double now_ms()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("usage: %s model.rknn image.jpg\n", argv[0]);
    return -1;
  }

  // ---------- 加载模型 ----------
  FILE *f = fopen(argv[1], "rb");
  if (!f)
  {
    printf("open model failed\n");
    return -1;
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
    return -1;
  }
  fclose(f);

  rknn_context ctx = 0;
  int ret = rknn_init(&ctx, model, (int)msz, 0, NULL);
  free(model);
  if (ret < 0)
  {
    printf("rknn_init failed %d\n", ret);
    return -1;
  }

  int ret_code = -1;
  bool outs_got = false;
  rknn_output *outs = NULL;

  float scale = 1.0f;
  int pad_w = 0, pad_h = 0;
  cv::Mat rgb;

  rknn_input_output_num io;
  memset(&io, 0, sizeof(io));

  rknn_tensor_attr in_attr;
  memset(&in_attr, 0, sizeof(in_attr));

  rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io, sizeof(io));
  in_attr.index = 0;
  rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &in_attr, sizeof(in_attr));

  const int MH = (int)in_attr.dims[1];
  const int MW = (int)in_attr.dims[2];
  printf("model loaded, input %dx%d (W x H), outputs=%u\n", MW, MH, io.n_output);

  if (io.n_output != 2)
  {
    printf("!! 这个程序假设 2 个输出（coords/scores），实际 %u 个\n", io.n_output);
    goto out_rknn;
  }

  // ---------- 读图 + 预处理 ----------
  {
    VideoCapture cap(argv[2]);
    if (!cap.isOpened())
    {
      printf("摄像头打开失败\n");
      return -1;
    }
    double fps = cap.get(CAP_PROP_FPS);
    if (fps < 1 || fps > 120)
      fps = 25;
    int w = (int)cap.get(CAP_PROP_FRAME_WIDTH);
    int h = (int)cap.get(CAP_PROP_FRAME_HEIGHT);
    int count = cap.get(CAP_PROP_FRAME_COUNT);
    printf("weight=%d height=%d fps=%f count=%d\n", w, h, fps, count);

    VideoWriter vm("out_video.mp4", VideoWriter::fourcc('m', 'p', '4', 'v'), fps, Size(w, h));
    if (!vm.isOpened())
    {
      printf("窗口打开失败\n");
      return -1;
    }
    outs = (rknn_output *)calloc(io.n_output, sizeof(rknn_output));
    Mat frame;
    Mat rgb;
    float scale;
    int pad_h, pad_w;
    int frames = 0;
    // rgb = read_and_letterbox(argv[2], MW, MH, &scale, &pad_w, &pad_h);
    double t0 = now_ms();
    double t_decode=0, t_run=0, t_get_out=0;
    while (cap.read(frame))
    {
      rgb = letterbox(frame, MW, MH, &scale, &pad_w, &pad_h);
      if (rgb.empty() || rgb.cols != MW || rgb.rows != MH) {
        printf("!! letterbox 返回空或尺寸不对\n");
        goto out_rknn;
      }
      if (!rgb.isContinuous())
        rgb = rgb.clone();
      {
        rknn_input in;
        memset(&in, 0, sizeof(in));
        in.index = 0;
        in.type = RKNN_TENSOR_UINT8;
        in.fmt = RKNN_TENSOR_NHWC;
        in.size = (uint32_t)(MW * MH * 3);
        in.buf = rgb.data;
        in.pass_through = 0;

        ret = rknn_inputs_set(ctx, 1, &in);
        if (ret < 0)
        {
          printf("inputs_set failed %d\n", ret);
          goto out_rknn;
        }
      }
      double t2 = now_ms();
      ret = rknn_run(ctx, NULL);
      double t3 = now_ms();
      t_run += t3 - t2;
      if (ret < 0)
      {
        printf("run failed\n");
        goto out_rknn;
      }
      if (!outs)
      {
        printf("calloc outs failed\n");
        goto out_rknn;
      }
      for (uint32_t i = 0; i < io.n_output; ++i)
      {
        outs[i].index = i;
        outs[i].want_float = 1;
      }
      ret = rknn_outputs_get(ctx, io.n_output, outs, NULL);
      if (ret < 0)
      {
        printf("outputs_get failed %d\n", ret);
        goto out_rknn;
      }
      outs_got = true;
      double t4 = now_ms();
      t_get_out += t4 - t3;
      // printf("infer %.2f ms,  outputs_get %.2f ms\n", t3 - t2, t4 - t3);
      const float *coords = (const float *)outs[0].buf;
      const float *scores = (const float *)outs[1].buf;
      const int NA = 8400, NC = 80;

      double t5 = now_ms();
      std::vector<Detection> dets = decode(coords, scores, NA, NC, 0.25f, 0.7f);
      double t6 = now_ms();
      t_decode += t6 - t5;

      for (auto det : dets)
      {
        char text[64];
        snprintf(text, sizeof(text), "cls=%d %.2f", det.cls, det.score);

        float x1 = (det.box.x1 - pad_w) / scale;
        float x2 = (det.box.x2 - pad_w) / scale;
        float y1 = (det.box.y1 - pad_h) / scale;
        float y2 = (det.box.y2 - pad_h) / scale;
        rectangle(frame, Rect(x1, y1, x2 - x1, y2 - y1), Scalar(0, 255, 0), 2);
        putText(frame, text, Point((int)x1, (int)y1 - 4), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);
      }
      vm.write(frame);
      rknn_outputs_release(ctx, io.n_output, outs);
      outs_got = false;
      frames++;
    }
    double elapsed = now_ms() - t0;
    printf("frames=%d, avg=%.2f ms, fps=%.2f\n, t_decode=%f, t_get_out=%f, t_run=%f",
           frames, elapsed / frames, frames * 1000.0 / elapsed, t_decode / frames * 1.0, t_get_out / frames * 1.0, t_run / frames * 1.0);
    free(outs);
    outs = NULL;
    rknn_destroy(ctx);
    ctx = 0;
    ret_code = 0;
  }

out_rknn:
  if (outs_got && ctx)
    rknn_outputs_release(ctx, io.n_output, outs);
  if (outs)
    free(outs);
  if (ctx)
    rknn_destroy(ctx);
  return ret_code;
}
