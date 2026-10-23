#include <cstdio>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <unordered_map>
struct Box
{
    float x1, y1, x2, y2;
};
struct Detection
{
    Box box;
    float score;
    int cls;
};

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
            printf("i=%d   scores[i]=%.2f   order[i]=%d\n", i, scores[i], order[i]);
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
std::vector<Detection> decode(std::vector<float> coords,
                              std::vector<float> scores,
                              float conf_thres,
                              float iou_thres)
{
    int num_anchors = coords.size() / 4;
    int num_classes = scores.size() / num_anchors;
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
static std::vector<float> read_bin(const char *path, size_t n)
{
    std::vector<float> v(n);
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        printf("!! cannot open %s\n", path);
        return {};
    }
    size_t got = fread(v.data(), sizeof(float), n, f);
    fclose(f);
    if (got != n)
    {
        printf("!! %s: got %zu of %zu\n", path, got, n);
        return {};
    }
    return v;
}

int main()
{
    std::vector<Box> boxes = {
        {0, 0, 100, 100},     // 0
        {5, 5, 105, 105},     // 1  ← 和 0 高度重叠
        {500, 500, 600, 600}, // 2  ← 独立
    };
    std::vector<float> scores = {0.9f, 0.8f, 0.7f};

    printf("IoU(0,1) = %.4f   期望 0.8223\n", iou(boxes[0], boxes[1]));
    printf("IoU(0,2) = %.4f   期望 0.0000\n", iou(boxes[0], boxes[2]));
    printf("\n");

    auto k1 = nms(boxes, scores, 0.7f);
    printf("thresh=0.7 -> 保留 %zu 个: ", k1.size());
    for (int i : k1)
        printf("%d ", i);
    printf("   期望 2 个 (0 2)\n");

    auto k2 = nms(boxes, scores, 0.9f);
    printf("thresh=0.9 -> 保留 %zu 个: ", k2.size());
    for (int i : k2)
        printf("%d ", i);
    printf("   期望 3 个 (0 1 2)\n");
    std::vector<Box> b2 = {{0, 0, 100, 100},     // 0  cls 41
                           {5, 5, 105, 105},     // 1  cls 41 (和0重叠)
                           {500, 500, 600, 600}, // 2  cls 73
                           {10, 10, 110, 110}};  // 3  cls 41
    std::vector<float> s2 = {0.9f, 0.8f, 0.7f, 0.6f};
    std::vector<int> cls2 = {41, 41, 73, 41};

    auto k3 = nms_class_aware(b2, s2, cls2, 0.7f);
    printf("\nclass_aware -> 保留 %zu 个: ", k3.size());
    for (int i : k3)
        printf("%d ", i);
    printf("   期望 3 个 (0 3 2)\n");

    // ---------------- 真实数据（Python dump 出来的）----------------
    auto rc = read_bin("coords.bin", 4 * 8400);
    auto rs = read_bin("scores.bin", 80 * 8400);
    if (!rc.empty() && !rs.empty())
    {
        auto real = decode(rc, rs, 0.25f, 0.7f);
        printf("\n真实数据 -> %zu 个   (期望 4 个: cup / cup / cell phone / book)\n",
               real.size());
        for (const auto &x : real)
            printf("  cls=%d  score=%.3f  box=(%.1f,%.1f,%.1f,%.1f)\n",
                   x.cls, x.score, x.box.x1, x.box.y1, x.box.x2, x.box.y2);
    }

    return 0;
}
