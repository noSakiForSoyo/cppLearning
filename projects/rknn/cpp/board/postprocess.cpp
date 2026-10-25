
#include "postprocess.hpp"

float iou(const Box &a, const Box &b)
{
    float x1 = std::max(a.x1, b.x1);
    float y1 = std::max(a.y1, b.y1);
    float x2 = std::min(a.x2, b.x2);
    float y2 = std::min(a.y2, b.y2);
    float w = x2 - x1;
    float h = y2 - y1;
    if (w < 0)
    {
        w = 0;
    }
    if (h < 0)
    {
        h = 0;
    }
    float inter = w * h;
    float a_box = (a.x2 - a.x1) * (a.y2 - a.y1);
    float b_box = (b.x2 - b.x1) * (b.y2 - b.y1);
    return inter / (a_box + b_box - inter + 1e-7);
}

std::vector<int> nms(const std::vector<Box> &boxes,
                     const std::vector<float> &scores,
                     float thresh)
{
    std::vector<int> order(boxes.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int x, int y)
              { return scores[x] > scores[y]; });
    std::vector<int> keep;
    while (!order.empty())
    {
        int cur = order[0];
        keep.emplace_back(cur);
        std::vector<int> rest;
        for (int i = 1; i != order.size(); i++)
        {
            if (iou(boxes[cur], boxes[order[i]]) <= thresh)
            {
                rest.emplace_back(order[i]);
            }
        }
        order = rest;
    }
    return keep;
}
std::vector<int> nms_class_aware(std::vector<Box> boxes,
                                 std::vector<float> scores,
                                 std::vector<int> cls,
                                 float thresh)
{
    std::vector<int> keep;
    std::vector<int> out;
    std::vector<int> type;
    for (auto c : cls)
    {
        if (std::find(type.begin(), type.end(), c) == type.end())
        {
            type.emplace_back(c);
        }
    }

    for (auto a : type)
    {
        std::vector<int> index;
        for (int i = 0; i != cls.size(); i++)
        {
            if (cls[i] == a)
            {
                index.emplace_back(i);
            }
        }
        std::vector<Box> sub_boxes;
        std::vector<float> sub_scores;
        std::for_each(index.begin(), index.end(), [&](int x)
                      {
                sub_boxes.emplace_back(boxes[x]);
                sub_scores.emplace_back(scores[x]); });
        keep = nms(sub_boxes, sub_scores, thresh);
        for (auto c : keep)
            out.emplace_back(index[c]);
    }
    return out;
}
std::vector<Detection> decode(const float* coords,
                              const float* scores,
                              int num_anchors,
                              int num_classes,
                              float conf_thres,
                              float iou_thres)
{
    // int num_anchors = coords.size() / 4;
    // int num_classes = scores.size() / num_anchors;
    std::vector<float> conf;
    std::vector<int> cls;
    std::vector<float> scores_t;
    std::vector<int> cls_t;
    Box xyxy;
    std::vector<Box> boxes;
    float cx, cy, w, h;

    for (int i = 0; i < num_anchors; i++)
    {
        float best = -1;
        size_t best_c = 0;
        for (int c = 0; c < num_classes; c++)
        {
            if (scores[c * num_anchors + i] > best)
            {
                best = scores[c * num_anchors + i];
                best_c = c;
            }
        }
        conf.push_back(best);
        cls.push_back(best_c);
    }

    for (int i = 0; i < num_anchors; i++)
    {
        if (conf[i] > conf_thres)
        {
            cx = coords[i];
            cy = coords[num_anchors + i];
            w = coords[2 * num_anchors + i];
            h = coords[3 * num_anchors + i];
            xyxy.x1 = cx - w / 2;
            xyxy.y1 = cy - h / 2;
            xyxy.x2 = cx + w / 2;
            xyxy.y2 = cy + h / 2;
            boxes.push_back(xyxy);
            scores_t.push_back(conf[i]);
            cls_t.push_back(cls[i]);
        }
    }
    std::vector<int> keep = nms_class_aware(boxes, scores_t, cls_t, iou_thres);
    std::vector<Detection> det;
    for (int i = 0; i < keep.size(); i++)
    {
        Detection d;
        d.box = boxes[keep[i]];
        d.cls = cls_t[keep[i]];
        d.score = scores_t[keep[i]];
        det.push_back(d);
    }
    return det;
}