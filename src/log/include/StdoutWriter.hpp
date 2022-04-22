#pragma once

#include <string.h>

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include "Writer.h"

namespace phase0
{
class StdoutWriter : public Writer
{
public:
    // TODO Integration Configuration
    StdoutWriter(const std::string basename) : Writer(basename), size_(1024 * 60), writen_(0) 
    {
        buffer_ = new char[size_]{'\0'};
        cur_ = size_;
    }
    ~StdoutWriter() { delete[] buffer_; }
    void append(const char* data, int32_t len) override
    {
        cur_ = cur_ - len;
        memcpy(buffer_ + cur_, data, len);
        writen_ += len;
    }
    void flush() override { std::cout << std::string(buffer_ + cur_, writen_) << std::endl << std::endl << std::endl; }
    uint32_t writtenBytes() const override { return writen_; }
    void reset() override
    {
        delete[] buffer_;
        buffer_ = new char[size_];
        writen_ = 0;
        cur_ = size_;
    }

private:
    char* buffer_;
    uint32_t writen_;
    uint32_t cur_;
    uint32_t size_;
};
}  // namespace phase0