#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>

#include "bounded_queue.hpp"
#include "sink.hpp"
#include "source.hpp"
#include "stage.hpp"
#include "frame.hpp"

class Pipeline
{
public:
    Pipeline(Source &source, Stage &stage, Sink &sink,
             std::size_t capacity = 3);
    bool run();
    void stop();

private:
    Source &source_;
    Stage &stage_;
    Sink &sink_;
    BoundedQueue<Frame> queue_;
    std::atomic<bool> stop_{false};
};
