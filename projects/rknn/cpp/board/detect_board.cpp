// detect_board.cpp —— 板端 YOLOv8n INT8 检测
//
// 编译（板子上）:
//   g++ -O2 -std=c++17 -o detect_board detect_board.cpp postprocess.cpp \
//       -I. -I/usr/include/opencv4 \
//       -L/usr/lib -lrknnrt \
//       -lopencv_core -lopencv_imgproc -lopencv_imgcodecs
//
// 用法:
//   ./detect_board yolov8n_int8.rknn test.jpg

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

#include "rknn_api.h"
#include "postprocess.hpp"

using namespace cv;

static double now_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

// =====================================================================
//  读图 + letterbox + RGB
// =====================================================================
static cv::Mat read_and_letterbox(const char *path,
                                  int dst_w, int dst_h,
                                  float *out_scale,
                                  int *out_pad_w, int *out_pad_h)
{
    Mat src = imread(path, IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << path << " 打开失败\n";
        return cv::Mat();
    }
    cvtColor(src, src, COLOR_BGR2RGB);

    float scale = (float)dst_w / std::max(src.cols, src.rows);
    int nw = (int)round(src.cols * scale);
    int nh = (int)round(src.rows * scale);

    Mat dst;
    resize(src, dst, Size(nw, nh));

    int pad_h = (dst_h - nh) / 2;
    int pad_w = (dst_w - nw) / 2;

    Mat out;
    copyMakeBorder(dst, out,
                   pad_h, dst_h - nh - pad_h,
                   pad_w, dst_w - nw - pad_w,
                   BORDER_CONSTANT, Scalar(114, 114, 114));

    *out_scale = scale;
    *out_pad_w = pad_w;
    *out_pad_h = pad_h;
    return out;
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        printf("usage: %s model.rknn image.jpg\n", argv[0]);
        return -1;
    }

    // ---------- 加载模型 ----------
    FILE *f = fopen(argv[1], "rb");
    if (!f) { printf("open model failed\n"); return -1; }
    fseek(f, 0, SEEK_END);
    long msz = ftell(f);
    fseek(f, 0, SEEK_SET);
    void *model = malloc(msz);
    if (fread(model, 1, msz, f) != (size_t)msz) {
        printf("read model failed\n");
        fclose(f);
        free(model);
        return -1;
    }
    fclose(f);

    rknn_context ctx = 0;
    int ret = rknn_init(&ctx, model, (int)msz, 0, NULL);
    free(model);
    if (ret < 0) { printf("rknn_init failed %d\n", ret); return -1; }

    int  ret_code = -1;
    bool outs_got = false;
    rknn_output *outs = NULL;

    float   scale = 1.0f;
    int     pad_w = 0, pad_h = 0;
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

    if (io.n_output != 2) {
        printf("!! 这个程序假设 2 个输出（coords/scores），实际 %u 个\n", io.n_output);
        goto out_rknn;
    }

    // ---------- 读图 + 预处理 ----------
    {
        double t0 = now_ms();
        rgb = read_and_letterbox(argv[2], MW, MH, &scale, &pad_w, &pad_h);
        double t1 = now_ms();
        printf("preprocess %.2f ms   scale=%.6f pad=(%d,%d)\n",
               t1 - t0, scale, pad_w, pad_h);
    }
    if (rgb.empty() || rgb.cols != MW || rgb.rows != MH) {
        printf("!! read_and_letterbox 返回空或尺寸不对\n");
        goto out_rknn;
    }
    if (!rgb.isContinuous())
        rgb = rgb.clone();

    // ---------- 推理 ----------
    {
        rknn_input in;
        memset(&in, 0, sizeof(in));
        in.index        = 0;
        in.type         = RKNN_TENSOR_UINT8;
        in.fmt          = RKNN_TENSOR_NHWC;
        in.size         = (uint32_t)(MW * MH * 3);
        in.buf          = rgb.data;
        in.pass_through = 0;

        ret = rknn_inputs_set(ctx, 1, &in);
        if (ret < 0) { printf("inputs_set failed %d\n", ret); goto out_rknn; }
    }

    {
        double t2 = now_ms();
        ret = rknn_run(ctx, NULL);
        double t3 = now_ms();
        if (ret < 0) { printf("rknn_run failed %d\n", ret); goto out_rknn; }

    
        outs = (rknn_output *)calloc(io.n_output, sizeof(rknn_output));
        if (!outs) { printf("calloc outs failed\n"); goto out_rknn; }
        for (uint32_t i = 0; i < io.n_output; ++i) {
            outs[i].index = i;
            outs[i].want_float = 1;
        }
        ret = rknn_outputs_get(ctx, io.n_output, outs, NULL);
        if (ret < 0) { printf("outputs_get failed %d\n", ret); goto out_rknn; }
        outs_got = true;

        double t4 = now_ms();
        printf("infer %.2f ms,  outputs_get %.2f ms\n", t3 - t2, t4 - t3);
    }

    // ---------- 后处理 ----------
    {
        const float *coords = (const float *)outs[0].buf;
        const float *scores = (const float *)outs[1].buf;
        const int NA = 8400, NC = 80;

        double t5 = now_ms();
        std::vector<Detection> dets = decode(coords, scores, NA, NC, 0.25f, 0.7f);
        double t6 = now_ms();
        printf("postprocess %.2f ms\n", t6 - t5);

        rknn_outputs_release(ctx, io.n_output, outs);
        outs_got = false;
        free(outs);
        outs = NULL;
        rknn_destroy(ctx);
        ctx = 0;
        ret_code = 0;

        cv::Mat src = cv::imread(argv[2]);
        if (src.empty()) { printf("reload src failed\n"); return -1; }
        printf("src %dx%d,  %zu boxes (640-scale)\n", src.cols, src.rows, dets.size());

        // -------------------------------------------------------------
        //  TODO ③-2  坐标反变换（640 → 原图）并打印
        //     x_orig = (x_640 - pad_w) / scale
        //     y_orig = (y_640 - pad_h) / scale
        //     再 clamp 到 [0, src.cols-1] / [0, src.rows-1]
        // -------------------------------------------------------------
        for(auto det : dets){
            char text[64];
            snprintf(text, sizeof(text), "cls=%d %.2f", det.cls, det.score);

            float x1 = (det.box.x1 - pad_w) / scale;
            float x2 = (det.box.x2 - pad_w) / scale;
            float y1 = (det.box.y1 - pad_h) / scale;
            float y2 = (det.box.y2 - pad_h) / scale;
            rectangle(src,Rect(x1,y1,x2-x1,y2-y1),Scalar(0,255,0),2);
            putText(src,text,Point((int)x1,(int)y1-4),FONT_HERSHEY_SIMPLEX,0.6,Scalar(0,0,255),2);
        }
        imwrite("out.jpg",src);
        // -------------------------------------------------------------
        //  TODO ③-3  画框 + 存图
        //     cv::rectangle(src, cv::Rect(x, y, w, h), cv::Scalar(0,255,0), 2);
        //     cv::putText(...);
        //     cv::imwrite("out.jpg", src);
        // -------------------------------------------------------------


        printf("\n[验收] 真机结果，反变换后应该落在这些位置附近:\n");
        printf("  cls=41  0.750   (853, 271) - (940, 396)\n");
        printf("  cls=67  0.279   (785, 648) - (823, 670)\n");
        printf("  cls=73  0.420   (687, 265) - (834, 412)\n");
        printf("  （按 scale=0.515713 pad=(0,86) 反算）\n");
    }

out_rknn:
    if (outs_got && ctx) rknn_outputs_release(ctx, io.n_output, outs);
    if (outs) free(outs);
    if (ctx) rknn_destroy(ctx);
    return ret_code;
}
