#include "Logger.h"

#include <AsynFileAppender.h>

#include <cstdarg>
#include <cstring>
#include <iostream>
#include <mutex>

namespace phase0
{
Logger::Logger() : level_(LogLevel::INFO), mutex_(), appender_(LogAppender::ptr(new AsynFileAppender("log")))
{
}

void Logger::setAppender(LogAppender::ptr appender)
{
    std::unique_lock<std::mutex> lock(mutex_);
    appender_ = appender;
}

void Logger::writeLog(
    LogLevel level, const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...)
{
    // TODO from config file
    if (level < level_)
    {
        return;
    }
    va_list ap;
    va_start(ap, format);
    // TODO buffer size
    char str[1024] = {0};
    int writenN = vsnprintf(str, 1024, format, ap);
    std::string res('\0', writenN);

    if (writenN > 0)
    {
        res.assign(str);
    }

    if (res.empty())
    {
        return;
    }

    // TODO formatter
    std::string prefix;
    prefix.append(Timestamp::now().toString());
    prefix.append(" ");
    prefix.append(getSourceFileName(fileName));
    prefix.append("|");
    prefix.append(functionName);
    prefix.append("|");
    prefix.append(std::to_string(lineNum) + " ");
    prefix.append("[" + getLogLevelStr(level) + "]");
    prefix.append(" => ");
    prefix.append(res);
    prefix.append("\n");

    {
        std::unique_lock<std::mutex> lock(mutex_);
        appender_->append(prefix.data(), prefix.size());
    }

    va_end(ap);
}

void Logger::setLevel(LogLevel level) { level_ = level; }

void Logger::info(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    writeLog(LogLevel::INFO, fileName, functionName, lineNum, format, ap);
    va_end(ap);
}

void Logger::debug(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    writeLog(LogLevel::DEBUG, fileName, functionName, lineNum, format, ap);
    va_end(ap);
}

void Logger::warn(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    writeLog(LogLevel::WARN, fileName, functionName, lineNum, format, ap);
    va_end(ap);
}

void Logger::error(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    writeLog(LogLevel::ERROR, fileName, functionName, lineNum, format, ap);
    va_end(ap);
}

void Logger::fatal(const char* fileName, const char* functionName, int32_t lineNum, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    writeLog(LogLevel::FATAL, fileName, functionName, lineNum, format, ap);
    va_end(ap);
}

static const char* getSourceFileName(const char* fileName)
{
    return strrchr(fileName, '/') ? strrchr(fileName, '/') + 1 : fileName;
};

static std::string getLogLevelStr(LogLevel level)
{
    switch (level)
    {
#define XX(name)         \
    case LogLevel::name: \
        return #name;    \
        break;

        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);
#undef XX
        default:
            return "UNKNOW";
    }
    return "UNKNOW";
}

}  // namespace phase0