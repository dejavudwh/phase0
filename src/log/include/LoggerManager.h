#pragma once

#include <map>
#include <mutex>
#include <string>

#include "singleton.h"
#include "Logger.h"

namespace phase0
{
class LoggerManager
{
public:
    LoggerManager();

    Logger::ptr getLogger(const std::string& name);

    Logger::ptr getRoot() const { return m_root; }

private:
    std::mutex m_mutex;
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

using LoggerManagerInstance = Singleton<LoggerManager>;

}  // namespace phase0