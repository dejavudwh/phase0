#include "AsynFileAppender.h"

#include <iostream>
#include <memory>
#include <mutex>

namespace phase0
{
AsynFileAppender::AsynFileAppender(const std::string& basename)
    : basename_(basename)
    , started_(false)
    , running_(false)
    , mutex_()
    , cond_()
    , latch_()
    , latchCond_()
    , threadWrapper(
          [&]() -> std::thread { return std::thread(std::bind(&AsynFileAppender::listenLogAppend, this)); })
    , curBuffer_(new LogBuffer())
{
    mkdir(basename.c_str(), 0755);
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
    std::unique_lock<std::mutex> lock(latch_);
    latchCond_.wait(lock);
}

void AsynFileAppender::stop()
{
    started_ = false;
    running_ = false;

    cond_.notify_one();
    persistThread_.join();
}

void AsynFileAppender::append(const char* data, size_t length)
{
    std::unique_lock<std::mutex> lock(mutex_);

    std::cout << "AsynFileAppender::append:" << data << std::endl;
    if (curBuffer_->available() >= length)
    {
        curBuffer_->append(data, length);
    }
    else
    {
        buffers_.push_back(std::move(curBuffer_));
        curBuffer_.reset(new LogBuffer());
        curBuffer_->append(data, length);
        cond_.notify_one();
    }
}

void AsynFileAppender::listenLogAppend()
{
    std::vector<std::unique_ptr<LogBuffer>> persistBuffers;
    // TODO from config
    LogFile logFile(basename_, 1000, 15);

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

            std::cout << "AsynFileAppender::push:" << curBuffer_->data() << std::endl;
            buffers_.push_back(std::move(curBuffer_));
            persistBuffers.swap(buffers_);
            curBuffer_.reset(new LogBuffer());
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