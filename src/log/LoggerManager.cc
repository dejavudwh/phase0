#include "LoggerManager.h"
#include "StdoutLogAppender.h"

#include <iostream>

namespace phase0
{
LoggerManager::LoggerManager()
{
    m_root.reset(new Logger);
    m_root->addAppender(LogAppender::ptr(new StdoutLogAppender()));

    m_loggers[m_root->m_name] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end())
    {
        return it->second;
    }

    Logger::ptr logger(new Logger(name));
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

}  // namespace phase0