#pragma once

#include <cstddef>
#include <string>

namespace phase0
{
class LogBuffer
{
public:
    // TODO size
    LogBuffer(size_t size = 200);
    ~LogBuffer();

    void append(const char* data, size_t length);
    void clear();

    const char* data() const;

    size_t available() const;
    size_t length() const;

private:
    char* data_;
    const size_t size_;
    size_t available_;
    size_t cur_;
};
}  // namespace phase0