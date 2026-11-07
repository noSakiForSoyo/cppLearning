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
std::size_t Pipeline::dropped() const
{
    return queue_.dropped();
}
void Pipeline::printStats()
{
    this->stage_.printStats();
    int frames = this->stage_.frames();
    double t_write = this->sink_.writeTime();
    double t_read = this->source_.readTime();
    printf("t_read=%f,t_write=%f\n", t_read / this->source_.produced(), t_write / frames);
    printf("dropped:%ld\n", queue_.dropped());
    printf("next_id:%ld\n", this->source_.produced());
}
void Pipeline::stop()
{
    stop_.store(true);
    queue_.stop();
}