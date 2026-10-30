#pragma once

#include <cstdio>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <cstdint>
#include <opencv2/core.hpp>
using namespace cv;
struct Box {
    float x1, y1, x2, y2;      
};

struct Detection {
    Box   box;
    float score;
    int   cls;
};

float iou(const Box &a, const Box &b);
std::vector<int> nms(const std::vector<Box> &boxes,
                     const std::vector<float> &scores,
                     float thresh);
std::vector<int> nms_class_aware(std::vector<Box> boxes,
                                 std::vector<float> scores,
                                 std::vector<int> cls,
                                 float thresh);                     
std::vector<Detection> decode(const float *coords,
                              const float *scores,
                              int   num_anchors,
                              int   num_classes,
                              float conf_thres,
                              float iou_thres);
std::vector<Detection> decode_int8(
    const int8_t *coords,
    const int8_t *scores,
    int num_anchors,
    int num_classes,
    int coord_zp,
    float coord_scale,
    int score_zp,
    float score_scale,
    float conf_thres,
    float iou_thres);
cv::Mat letterbox(const cv::Mat &bgr,
                        int dst_w,int dst_h,
                        float *out_scale,
                        int *out_pad_w,int *out_pad_h);