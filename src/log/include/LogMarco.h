#pragma once

#include <assert.h>

#include "Logger.h"
#include "LoggerManager.h"
#include "utils.h"

#define PHASE0_LOG_LEVEL(logger, level)                                                           \
    if (logger->getLevel() <= level)                                                              \
    phase0::LogEventWrap(phase0::LogEvent::ptr(new phase0::LogEvent(logger,                       \
                                                                    level,                        \
                                                                    __FILE__,                     \
                                                                    __LINE__,                     \
                                                                    0,                            \
                                                                    phase0::GetCurThreadId(),     \
                                                                    phase0::GetCurFiberId(),      \
                                                                    time(0),                      \
                                                                    phase0::GetCurThreadName()))) \
        .getSS()

#define PHASE0_LOG_DEBUG(logger) PHASE0_LOG_LEVEL(logger, phase0::LogLevel::DEBUG)
#define PHASE0_LOG_INFO(logger) PHASE0_LOG_LEVEL(logger, phase0::LogLevel::INFO)
#define PHASE0_LOG_WARN(logger) PHASE0_LOG_LEVEL(logger, phase0::LogLevel::WARN)
#define PHASE0_LOG_ERROR(logger) PHASE0_LOG_LEVEL(logger, phase0::LogLevel::ERROR)
#define PHASE0_LOG_FATAL(logger) PHASE0_LOG_LEVEL(logger, phase0::LogLevel::FATAL)

#define PHASE0_LOG_FMT_LEVEL(logger, level, fmt, ...)                                            \
    if (logger->getLevel() <= level)                                                             \
    phase0::LogEventWrap(phase0::LogEvent::ptr(new phase0::LogEvent(logger,                      \
                                                                    level,                       \
                                                                    __FILE__,                    \
                                                                    __LINE__,                    \
                                                                    0,                           \
                                                                    phase0::GetThreadId(),       \
                                                                    phase0::GetFiberId(),        \
                                                                    time(0),                     \
                                                                    phase0::Thread::GetName()))) \
        .getEvent()                                                                              \
        ->format(fmt, __VA_ARGS__)

#define PHASE0_LOG_FMT_DEBUG(logger, fmt, ...) \
    PHASE0_LOG_FMT_LEVEL(logger, phase0::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define PHASE0_LOG_FMT_INFO(logger, fmt, ...) \
    PHASE0_LOG_FMT_LEVEL(logger, phase0::LogLevel::INFO, fmt, __VA_ARGS__)
#define PHASE0_LOG_FMT_WARN(logger, fmt, ...) \
    PHASE0_LOG_FMT_LEVEL(logger, phase0::LogLevel::WARN, fmt, __VA_ARGS__)
#define PHASE0_LOG_FMT_ERROR(logger, fmt, ...) \
    PHASE0_LOG_FMT_LEVEL(logger, phase0::LogLevel::ERROR, fmt, __VA_ARGS__)
#define PHASE0_LOG_FMT_FATAL(logger, fmt, ...) \
    PHASE0_LOG_FMT_LEVEL(logger, phase0::LogLevel::FATAL, fmt, __VA_ARGS__)

#define PHASE0_LOG_ROOT() phase0::LoggerManagerInstance::GetInstance()->getRoot()
#define PHASE0_LOG_SYSTEM() phase0::LoggerManagerInstance::GetInstance()->getLogger("system")

#define PHASE0_LOG_NAME(name) phase0::LoggerManagerInstance::GetInstance()->getLogger(name)

#define P0ROOT_LOG_DEBUG() PHASE0_LOG_LEVEL(PHASE0_LOG_ROOT(), phase0::LogLevel::DEBUG)
#define P0ROOT_LOG_INFO() PHASE0_LOG_LEVEL(PHASE0_LOG_ROOT(), phase0::LogLevel::INFO)
#define P0ROOT_LOG_WARN() PHASE0_LOG_LEVEL(PHASE0_LOG_ROOT(), phase0::LogLevel::WARN)
#define P0ROOT_LOG_ERROR() PHASE0_LOG_LEVEL(PHASE0_LOG_ROOT(), phase0::LogLevel::ERROR)
#define P0ROOT_LOG_FATAL() PHASE0_LOG_LEVEL(PHASE0_LOG_ROOT(), phase0::LogLevel::FATAL)

#define P0SYS_LOG_DEBUG() PHASE0_LOG_LEVEL(PHASE0_LOG_SYSTEM(), phase0::LogLevel::DEBUG)
#define P0SYS_LOG_INFO() PHASE0_LOG_LEVEL(PHASE0_LOG_SYSTEM(), phase0::LogLevel::INFO)
#define P0SYS_LOG_WARN() PHASE0_LOG_LEVEL(PHASE0_LOG_SYSTEM(), phase0::LogLevel::WARN)
#define P0SYS_LOG_ERROR() PHASE0_LOG_LEVEL(PHASE0_LOG_SYSTEM(), phase0::LogLevel::ERROR)
#define P0SYS_LOG_FATAL() PHASE0_LOG_LEVEL(PHASE0_LOG_SYSTEM(), phase0::LogLevel::FATAL)

// common marcho

#if defined __GNUC__ || defined __llvm__
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率成立
#define PHASE0_LIKELY(x) __builtin_expect(!!(x), 1)
/// LIKCLY 宏的封装, 告诉编译器优化,条件大概率不成立
#define PHASE0_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define PHASE0_LIKELY(x) (x)
#define PHASE0_UNLIKELY(x) (x)
#endif

/// 断言宏封装
#define PHASE0_ASSERT(x)                                                \
    if (PHASE0_UNLIKELY(!(x)))                                           \
    {                                                                   \
        P0SYS_LOG_FATAL() << "ASSERTION: " #x << "\nbacktrace:\n"      \
                           << phase0::BacktraceToString(100, 2, "    "); \
        assert(x);                                                      \
    }

/// 断言宏封装
#define PHASE0_ASSERT2(x, w)                                            \
    if (PHASE0_UNLIKELY(!(x)))                                           \
    {                                                                   \
        P0SYS_LOG_FATAL() << "ASSERTION: " #x << "\n"                  \
                           << w << "\nbacktrace:\n"                     \
                           << phase0::BacktraceToString(100, 2, "    "); \
        assert(x);                                                      \
    }