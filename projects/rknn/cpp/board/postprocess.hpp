#pragma once

#include <cstdio>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <unordered_map>

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
