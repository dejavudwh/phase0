#pragma once

#include <fstream>
#include <memory>

#include "LogLevel.h"
#include "Logger.h"

namespace phase0
{
class FileLogAppender : public LogAppender
{
public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& filename);
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

    bool reopen();

private:
    std::string m_filename;
    std::ofstream m_filestream;

    uint64_t m_lastTime = 0;
};

}  // namespace phase0