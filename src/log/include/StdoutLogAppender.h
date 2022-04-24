#pragma once

#include "Logger.h"

namespace phase0
{
class StdoutLogAppender : public LogAppender
{
public:
    using ptr = std::shared_ptr<StdoutLogAppender>;
    void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
};

}  // namespace phase0