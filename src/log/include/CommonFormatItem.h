#include <map>
#include <string>
#include <tuple>
#include <utility>

#include "LogLevel.h"
#include "Logger.h"

namespace phase0
{
using ColorConsolePair = std::pair<std::string, std::string>;

ColorConsolePair a{"", ""};
// UNKNOW = 0,
// DEBUG = 1,
// INFO = 2,
// WARN = 3,
// ERROR = 4,
// FATAL = 5

static std::vector<ColorConsolePair> colorConsolePairVec = {
    ColorConsolePair{"", ""},                 // UNKNOW
    ColorConsolePair{"\033[34m", "\033[0m"},  // DEBUG
    ColorConsolePair{"\033[32m", "\033[0m"},  // INFO
    ColorConsolePair{"\033[33m", "\033[0m"},  // WARN
    ColorConsolePair{"\033[31m", "\033[0m"},  // ERROR
    ColorConsolePair{"\033[35m", "\033[0m"},  // FATAL
};

class MessageFormatItem : public LogFormatter::FormatItem
{
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getContent() << colorConsolePairVec[level].second;
    }
};

class LevelFormatItem : public LogFormatter::FormatItem
{
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << LogLevel::ToString(level)
           << colorConsolePairVec[level].second;
    }
};

class ElapseFormatItem : public LogFormatter::FormatItem
{
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getElapse() << colorConsolePairVec[level].second;
    }
};

class NameFormatItem : public LogFormatter::FormatItem
{
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getLogger()->getName()
           << colorConsolePairVec[level].second;
    }
};

class ThreadIdFormatItem : public LogFormatter::FormatItem
{
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getThreadId() << colorConsolePairVec[level].second;
    }
};

class FiberIdFormatItem : public LogFormatter::FormatItem
{
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getFiberId() << colorConsolePairVec[level].second;
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem
{
public:
    ThreadNameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getThreadName() << colorConsolePairVec[level].second;
    }
};

class DateTimeFormatItem : public LogFormatter::FormatItem
{
public:
    DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : m_format(format)
    {
        if (m_format.empty())
        {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }

    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << colorConsolePairVec[level].first << buf << colorConsolePairVec[level].second;
    }

private:
    std::string m_format;
};

class FilenameFormatItem : public LogFormatter::FormatItem
{
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getFile() << colorConsolePairVec[level].second;
    }
};

class LineFormatItem : public LogFormatter::FormatItem
{
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << event->getLine() << colorConsolePairVec[level].second;
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem
{
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << std::endl << colorConsolePairVec[level].second;
    }
};

class StringFormatItem : public LogFormatter::FormatItem
{
public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << m_string << colorConsolePairVec[level].second;
    }

private:
    std::string m_string;
};

class TabFormatItem : public LogFormatter::FormatItem
{
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override
    {
        os << colorConsolePairVec[level].first << "\t" << colorConsolePairVec[level].second;
    }

private:
    std::string m_string;
};

}  // namespace phase0