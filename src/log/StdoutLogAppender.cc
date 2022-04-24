#include "StdoutLogAppender.h"

#include <memory>
#include <iostream>

namespace phase0
{
void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_formatter->format(std::cout, logger, level, event);
    }
}

}  // namespace phase0