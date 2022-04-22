#pragma once

#include <cstdarg>
#include <cstdlib>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "LogAppender.h"
#include "Timestamp.h"

namespace phase0
{
#define LOG_INFO(format, ...)                                                                             \
    do                                                                                                    \
    {                                                                                                     \
        phase0::Logger& logger = phase0::Logger::getInstance();                                           \
        logger.writeLog(phase0::LogLevel::INFO, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
    } while (0)

#define LOG_DEBUG(format, ...)                                                                             \
    do                                                                                                     \
    {                                                                                                      \
        phase0::Logger& logger = phase0::Logger::getInstance();                                            \
        logger.writeLog(phase0::LogLevel::DEBUG, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
    } while (0)

#define LOG_WARN(format, ...)                                                                             \
    do                                                                                                    \
    {                                                                                                     \
        phase0::Logger& logger = phase0::Logger::getInstance();                                           \
        logger.writeLog(phase0::LogLevel::WARN, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
    } while (0)

#define LOG_ERROR(format, ...)                                                                             \
    do                                                                                                     \
    {                                                                                                      \
        phase0::Logger& logger = phase0::Logger::getInstance();                                            \
        logger.writeLog(phase0::LogLevel::ERROR, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
    } while (0)

#define LOG_FATAL(format, ...)                                                                             \
    do                                                                                                     \
    {                                                                                                      \
        phase0::Logger& logger = phase0::Logger::getInstance();                                            \
        logger.writeLog(phase0::LogLevel::FATAL, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__); \
    } while (0)

enum class LogLevel
{
    INFO,
    DEBUG,
    WARN,
    ERROR,
    FATAL,
};
class Logger
{
public:
    static Logger& getInstance()
    {
        static Logger logger;
        return logger;
    }

    void setAppender(LogAppender::ptr appender);
    void writeLog(LogLevel level,
                  const char* fileName,
                  const char* functionName,
                  int32_t lineNum,
                  const char* format,
                  ...);
    void setLevel(LogLevel level);

    void info(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...);
    void debug(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...);
    void warn(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...);
    void error(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...);
    void fatal(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...);

    static void initDefaultConfig();

private:
    Logger();
    std::mutex mutex_;
    LogLevel level_;
    LogAppender::ptr appender_;
};

static std::string getLogLevelStr(LogLevel level);
static const char* getSourceFileName(const char* fileName);
}  // namespace phase0