#include "Logger.h"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>

#include "CommonFormatItem.h"

namespace phase0
{
LogEvent::LogEvent(std::shared_ptr<Logger> logger,
                   LogLevel::Level level,
                   const char* file,
                   int32_t line,
                   uint32_t elapse,
                   uint32_t threadId,
                   uint32_t fiberId,
                   uint64_t time,
                   const std::string& threadName)
    : m_file(file)
    , m_line(line)
    , m_elapse(elapse)
    , m_threadId(threadId)
    , m_fiberId(fiberId)
    , m_time(time)
    , m_threadName(threadName)
    , m_logger(logger)
    , m_level(level)
{
}

LogEventWrap::LogEventWrap(LogEvent::ptr e) : m_event(e) {}

LogEventWrap::~LogEventWrap()
{
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

void LogEvent::format(const char* fmt, ...)
{
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al)
{
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1)
    {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) { init(); }

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event)
{
    std::stringstream ss;
    for (auto& i : m_items)
    {
        i->format(ss, logger, level, event);
    }
    return ss.str();
}

std::ostream& LogFormatter::format(std::ostream& ofs,
                                   std::shared_ptr<Logger> logger,
                                   LogLevel::Level level,
                                   LogEvent::ptr event)
{
    for (auto& i : m_items)
    {
        i->format(ofs, logger, level, event);
    }
    return ofs;
}

void LogAppender::setFormatter(LogFormatter::ptr val)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_formatter = val;
    if (m_formatter)
    {
        m_hasFormatter = true;
    }
    else
    {
        m_hasFormatter = false;
    }
}

LogFormatter::ptr LogAppender::getFormatter()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_formatter;
}

LogFormatter::ptr Logger::getFormatter()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_formatter;
}

void LogFormatter::init()
{
    // str, format, type
    // %d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n
    std::vector<std::tuple<std::string, std::string, int> > vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i)
    {
        // const
        if (m_pattern[i] != '%')
        {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        // %%
        if ((i + 1) < m_pattern.size())
        {
            if (m_pattern[i + 1] == '%')
            {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmtStatus = 0;
        size_t fmtBegin = 0;

        std::string str;
        std::string fmt;
        while (n < m_pattern.size())
        {
            // const !alpha
            if (!fmtStatus && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}'))
            {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmtStatus == 0)
            {
                if (m_pattern[n] == '{')
                {
                    str = m_pattern.substr(i + 1, n - i - 1);  // FormatterItem
                    fmtStatus = 1;                             // start parse fmt
                    fmtBegin = n;
                    ++n;
                    continue;
                }
            }
            else if (fmtStatus == 1)
            {
                if (m_pattern[n] == '}')
                {
                    fmt = m_pattern.substr(fmtBegin + 1, n - fmtBegin - 1);  // fmt
                    fmtStatus = 0;                                           // fmt end
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size())
            {
                if (str.empty())
                {
                    str = m_pattern.substr(i + 1);  // % d
                }
            }
        }

        if (fmtStatus == 0)
        {
            if (!nstr.empty())
            {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
        else if (fmtStatus == 1)
        {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }

    if (!nstr.empty())
    {
        vec.push_back(std::make_tuple(nstr, "", 0));
    }

    static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)> > s_format_items = {
#define XX(str, C)                                                               \
    {                                                                            \
#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); } \
    }

        XX(m, MessageFormatItem),     // m:消息
        XX(p, LevelFormatItem),       // p:日志级别
        XX(r, ElapseFormatItem),      // r:累计毫秒数
        XX(c, NameFormatItem),        // c:日志名称
        XX(t, ThreadIdFormatItem),    // t:线程id
        XX(n, NewLineFormatItem),     // n:换行
        XX(d, DateTimeFormatItem),    // d:时间
        XX(f, FilenameFormatItem),    // f:文件名
        XX(l, LineFormatItem),        // l:行号
        XX(T, TabFormatItem),         // T:Tab
        XX(F, FiberIdFormatItem),     // F:协程id
        XX(N, ThreadNameFormatItem),  // N:线程名称
#undef XX
    };

    for (auto& i : vec)
    {
        if (std::get<2>(i) == 0)
        {
            m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
        }
        else
        {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end())
            {
                m_items.push_back(
                    FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                m_error = true;
            }
            else
            {
                m_items.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

Logger::Logger(const std::string& name) : m_name(name), m_level(LogLevel::DEBUG)
{
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l => %m%n"));
}

void Logger::setFormatter(LogFormatter::ptr val)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_formatter = val;

    for (auto& i : m_appenders)
    {
        std::unique_lock<std::mutex> llock(i->m_mutex);
        if (!i->m_hasFormatter)
        {
            i->m_formatter = m_formatter;
        }
    }
}

void Logger::setFormatter(const std::string& val)
{
    LogFormatter::ptr newVal(new LogFormatter(val));
    if (newVal->isError())
    {
        std::cout << "Logger setFormatter name=" << m_name << " value=" << val << " invalid formatter"
                  << std::endl;
        return;
    }
    setFormatter(newVal);
}

void Logger::addAppender(LogAppender::ptr appender)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!appender->getFormatter())
    {
        std::unique_lock<std::mutex> llock(appender->m_mutex);
        appender->m_formatter = m_formatter;
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
    {
        if (*it == appender)
        {
            m_appenders.erase(it);
            break;
        }
    }
}

void Logger::clearAppenders()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_appenders.clear();
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event)
{
    if (level >= m_level)
    {
        auto self = shared_from_this();
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_appenders.empty())
        {
            for (auto& i : m_appenders)
            {
                i->log(self, level, event);
            }
        }
        else if (m_root)
        {
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }

void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }

void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }

void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }

void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

}  // namespace phase0