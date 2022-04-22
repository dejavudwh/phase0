#include "AsynFileAppender.h"

#include <iostream>
#include <memory>
#include <mutex>

namespace phase0
{
AsynFileAppender::AsynFileAppender(int bufferSize) :
    LogAppender()
    , started_(false)
    , running_(false)
    , mutex_()
    , cond_()
    , latch_()
    , latchCond_()
    , threadWrapper(
          [&]() -> std::thread { return std::thread(std::bind(&AsynFileAppender::listenLogAppend, this)); })
    , curBuffer_(new LogBuffer(bufferSize))
    , bufferSize_(bufferSize)
{
    start();
}

AsynFileAppender::~AsynFileAppender()
{
    if (started_)
    {
        stop();
    }
}

void AsynFileAppender::start()
{
    started_ = true;
    running_ = true;

    persistThread_ = threadWrapper();
    {
        std::unique_lock<std::mutex> lock(latch_);
        latchCond_.wait(lock);
    }
    persistThread_.detach();
}

void AsynFileAppender::stop()
{
    started_ = false;
    running_ = false;

    cond_.notify_one();
}

void AsynFileAppender::append(const char* data, size_t length)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (curBuffer_->available() >= length)
    {
        curBuffer_->append(data, length);
    }
    else
    {
        buffers_.push_back(std::move(curBuffer_));
        curBuffer_.reset(new LogBuffer(bufferSize_));
        curBuffer_->append(data, length);
        cond_.notify_one();
    }
}

void AsynFileAppender::listenLogAppend()
{
    std::vector<std::unique_ptr<LogBuffer>> persistBuffers;
    // TODO Integration Configuration
    LogFile logFile(basename_, 10240, 5);

    latchCond_.notify_one();

    while (running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty())
            {
                cond_.wait(lock);
            }
            if (curBuffer_->length() == 0)
            {
                continue;
            }

            buffers_.push_back(std::move(curBuffer_));
            persistBuffers.swap(buffers_);
            buffers_.clear();
            curBuffer_.reset(new LogBuffer(bufferSize_));
            curBuffer_->clear();
        }

        for (const auto& it : persistBuffers)
        {
            logFile.append(it->data(), it->length());
        }

        persistBuffers.clear();

        if (!started_)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (curBuffer_->length() == 0)
            {
                running_ = false;
            }
        }
    }
}

}  // namespace phase0