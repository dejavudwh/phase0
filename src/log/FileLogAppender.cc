#include "FileLogAppender.h"
#include "utils.h"

#include <iostream>
#include <mutex>

namespace phase0
{
FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) { reopen(); }

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level)
    {
        uint64_t now = event->getTime();
        if (now >= (m_lastTime + 3))
        {
            reopen();
            m_lastTime = now;
        }
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_formatter->format(m_filestream, logger, level, event))
        {
            std::cout << "error" << std::endl;
        }
    }
}

bool FileLogAppender::reopen()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_filestream)
    {
        m_filestream.close();
    }
    return FSUtil::OpenForWrite(m_filestream, m_filename, std::ios::app);
}

}  // namespace phase0