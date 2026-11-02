#include "../pipeline/stage.hpp"

class ResizeStage : public Stage
{
public:
    explicit ResizeStage(cv::Size out_size);
    bool process(Frame &frame) override;

private:
    cv::Size out_size_;
};