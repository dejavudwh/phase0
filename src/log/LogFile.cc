#include "LogFile.h"

#include <cstdint>
#include <memory>
#include <mutex>

#include "StdoutWriter.hpp"
#include "Timestamp.h"
#include "Writer.h"

namespace phase0
{
const int64_t oneDayMicroSeconds = 86400000;

LogFile::LogFile(const std::string& basename, int32_t rollSize, int32_t flushIntervel)
    : basename_(basename)
    , rollSize_(rollSize)
    , flushIntervel_(flushIntervel)
    , lastFlush_(Timestamp::now().microSeconds())
    , lastRoll_(Timestamp::now().microSeconds())
{
    // TEST
    writers_["stdout"] = Writer::ptr(new StdoutWriter());
}

LogFile::~LogFile() {}

void LogFile::append(const char* log, int32_t length)
{
    for (auto& it : writers_)
    {
        auto& writer = it.second;
        writer->append(log, length);
        std::cout << writer->writtenBytes() << " <===> " << rollSize_ + length << std::endl;
        if (writer->writtenBytes() >= rollSize_ + length)
        {
            rollFile();
        }
        else
        {
            int64_t now = Timestamp::now().microSeconds();
            if (now - lastFlush_ >= flushIntervel_)
            {
                std::cout << "Flush Intervel Time:" << now - lastFlush_ << std::endl;
                writer->flush();
                lastFlush_ = now;
            }
            else if (now - lastRoll_ >= oneDayMicroSeconds)
            {
                rollFile();
                lastRoll_ = now;
            }
        }
    }
}

void LogFile::flush()
{
    for (auto& it : writers_)
    {
        it.second->flush();
    }
}

void LogFile::rollFile()
{
    for (auto& it : writers_)
    {
        it.second->flush();
        it.second->reset();
    }
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