#pragma once

#include <sys/stat.h>

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "LogAppender.h"
#include "LogBuffer.h"
#include "LogFile.h"

namespace phase0
{
class AsynFileAppender : public LogAppender
{
public:
    AsynFileAppender(int bufferSize);
    ~AsynFileAppender();
    void append(const char* data, size_t length);
    void start();
    void stop();
    int getBufferSize();

private:
    void listenLogAppend();

    std::string basename_;

    bool started_;
    bool running_;

    std::mutex mutex_;
    std::condition_variable cond_;
    std::mutex latch_;
    std::condition_variable latchCond_;
    std::function<std::thread()> threadWrapper;
    std::thread persistThread_;

    std::unique_ptr<LogBuffer> curBuffer_;
    std::vector<std::unique_ptr<LogBuffer>> buffers_;
    int bufferSize_;
};
}  // namespace phase0