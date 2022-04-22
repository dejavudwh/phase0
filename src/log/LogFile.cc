#include "LogFile.h"

#include <cstdint>
#include <memory>
#include <mutex>

#include "MMapFileWriter.hpp"
#include "StdoutWriter.hpp"
#include "Timestamp.h"
#include "Writer.h"

namespace phase0
{
const int64_t oneDayMicroSeconds = 86400000;

LogFile::LogFile(const std::string& basename, int32_t rollSize, int32_t flushInterval)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushInterval_(flushInterval)
    , lastFlush_(Timestamp::now().microSeconds())
    , lastRoll_(Timestamp::now().microSeconds())
{
    // TODO Integration Configuration
    writers_["mmap"] = Writer::ptr(new MMapFileWriter("log"));
    writers_["stdout"] = Writer::ptr(new StdoutWriter(""));
}

LogFile::~LogFile() {}

void LogFile::append(const char* log, int32_t length)
{
    int isRollNum = 0;
    for (auto& it : writers_)
    {
        auto& writer = it.second;
        writer->append(log, length);
        if (writer->writtenBytes() >= rollSize_ + length)
        {
            rollFile(writer);
            isRollNum++;
        }
        else
        {
            int64_t now = Timestamp::now().microSeconds();
            if (now - lastFlush_ >= flushInterval_)
            {
                flush(writer);
                lastFlush_ = now;
            }
            else if (now - lastRoll_ >= oneDayMicroSeconds)
            {
                rollFile(writer);
                lastRoll_ = now;
            }
        }
    }
}

void LogFile::flush(std::shared_ptr<Writer>& writer) { writer->flush(); }

void LogFile::rollFile(std::shared_ptr<Writer>& writer)
{
    writer->flush();
    writer->reset();
}

void LogFile::addWriter(const std::string& name, Writer::ptr& writer)
{
    std::unique_lock<std::mutex> lock(mutex_);
    writers_[name] = writer;
}

void LogFile::deleteWriter(const std::string& name)
{
    auto it = writers_.find(name);
    if (it != writers_.end())
    {
        writers_.erase(it);
    }
}
void LogFile::clearWriter() { writers_.clear(); }

}  // namespace phase0