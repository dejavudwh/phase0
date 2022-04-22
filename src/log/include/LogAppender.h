#pragma once

#include <memory>
#include <string>

namespace phase0
{
class LogAppender
{
public:
    using ptr = std::shared_ptr<LogAppender>;

    LogAppender(){};
    virtual ~LogAppender(){};
    virtual void append(const char* data, size_t length) = 0;
};
}  // namespace phase0