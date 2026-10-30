// detect_board.cpp —— 板端 YOLOv8n INT8 检测
//
// 编译（板子上）:
// g++ -O2 -std=c++17 -o detect_camera_int8 detect_camera_int8.cpp postprocess.cpp \
//   -I. -I/usr/include/opencv4 -L/usr/lib -lrknnrt \
//   -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_videoio
// 用法:
//   ./detect_camera_int8 yolov8n_int8.rknn

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
const int max_frames = 300;
static double now_ms()
{
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

int main(int argc, char **argv)
{
  if (argc < 2)
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
    printf("!! 这个程序假设 2 个输出(coords/scores),实际 %u 个\n", io.n_output);
    goto out_rknn;
  }
  rknn_tensor_attr out_attrs[2];
  memset(out_attrs, 0, sizeof(out_attrs));

  for (uint32_t i = 0; i < io.n_output; ++i)
  {
    out_attrs[i].index = i;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR,
                     &out_attrs[i], sizeof(rknn_tensor_attr));
    if (ret < 0)
    {
      printf("query output %u failed %d\n", i, ret);
      goto out_rknn;
    }

    printf("out %u name=%s dims=[%u,%u,%u] zp=%d scale=%f size=%u\n",
           i, out_attrs[i].name,
           out_attrs[i].dims[0],
           out_attrs[i].dims[1],
           out_attrs[i].dims[2],
           out_attrs[i].zp,
           out_attrs[i].scale,
           out_attrs[i].size);
  }
  // ---------- 读图 + 预处理 ----------
  {
    VideoCapture cap("/dev/video10", CAP_V4L2);
    if (!cap.isOpened())
    {
      printf("摄像头打开失败\n");
      return -1;
    }
    cap.set(CAP_PROP_FOURCC, VideoWriter::fourcc('M', 'J', 'P', 'G'));
    cap.set(CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(CAP_PROP_FPS, 30);
    printf("camera %dx%d fps=%.2f\n",
           (int)cap.get(CAP_PROP_FRAME_WIDTH),
           (int)cap.get(CAP_PROP_FRAME_HEIGHT),
           cap.get(CAP_PROP_FPS));
    std::string pipeline =
        "appsrc ! videoconvert ! "
        "video/x-raw,format=NV12,width=1280,height=720,framerate=7/1 ! "
        "mpph264enc ! h264parse ! mp4mux ! filesink location=out_cam_int8.mp4";
    VideoWriter vm(pipeline, CAP_GSTREAMER, 0, 7, Size(1280, 720));
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
    double t_decode = 0, t_run = 0, t_get_out = 0, t_read = 0, t_letter = 0, t_input = 0, t_write = 0;
    while (frames < max_frames)
    {
      double p0 = now_ms();
      if (!cap.read(frame))
      {
        break;
      };
      t_read += now_ms() - p0;
      double p1 = now_ms();
      rgb = letterbox(frame, MW, MH, &scale, &pad_w, &pad_h);
      t_letter += now_ms() - p1;
      if (rgb.empty() || rgb.cols != MW || rgb.rows != MH)
      {
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
        double p2 = now_ms();
        ret = rknn_inputs_set(ctx, 1, &in);
        t_input += now_ms() - p2;
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
        outs[i].want_float = 0;
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
      const int8_t *coords = (const int8_t *)outs[0].buf;
      const int8_t *scores = (const int8_t *)outs[1].buf;
      const int NA = 8400, NC = 80;

      double t5 = now_ms();
      std::vector<Detection> dets =
          decode_int8(coords, scores,  NA, NC, out_attrs[0].zp, out_attrs[0].scale, out_attrs[1].zp, out_attrs[1].scale, 0.25f, 0.7f);
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
      int p3 = now_ms();

      vm.write(frame);

      t_write += now_ms() - p3;
      rknn_outputs_release(ctx, io.n_output, outs);
      outs_got = false;
      frames++;
    }
    double elapsed = now_ms() - t0;
    printf("frames=%d, avg=%.2f ms, fps=%.2f\n, t_decode=%f, t_get_out=%f, t_run=%f,t_read=%f,t_letter=%f,t_input=%f,t_write=%f",
           frames, elapsed / frames,
           frames * 1000.0 / elapsed,
           t_decode / frames, t_get_out / frames, t_run / frames, t_read / frames, t_letter / frames, t_input / frames, t_write / frames);
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
