#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

#include "Writer.h"

namespace phase0
{
class LogFile
{
public:
    LogFile(const std::string& basename, int32_t rollSize, int32_t flushIntervel);
    ~LogFile();

    void append(const char* log, int32_t size);
    void flush(std::shared_ptr<Writer>& writer);
    void rollFile(std::shared_ptr<Writer>& writer);

    void addWriter(const std::string& name, Writer::ptr& Writer);
    void deleteWriter(const std::string& name);
    void clearWriter();

private:
    std::string basename_;

    int64_t rollSize_;
    int64_t flushIntervel_;
    int64_t lastFlush_;
    int64_t lastRoll_;

    std::mutex mutex_;

    std::unordered_map<std::string, Writer::ptr> writers_;
};

}  // namespace phase0