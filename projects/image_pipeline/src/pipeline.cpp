#include "pipeline/pipeline.hpp"

Pipeline::Pipeline(Source &source, Stage &stage, Sink &sink, std::size_t cap)
    : source_(source), stage_(stage), sink_(sink), queue_(cap) {}
bool Pipeline::run()
{
    std::thread producer([this]
                         {
            Frame frame;

            while (!stop_.load() && source_.read(frame))
            {
                if (!queue_.push(std::move(frame)))
                    break;
            }

            queue_.stop(); });
    Frame frame;
    bool ok = true;
    while (queue_.pop(frame))
    {
        if (!stage_.process(frame) || !sink_.write(frame))
        {
            ok = false;
            stop_.store(true);
            queue_.stop();
            break;
        }
    }
    producer.join();
    return ok;
}
void Pipeline::stop()
{
    stop_.store(true);
    queue_.stop();
}