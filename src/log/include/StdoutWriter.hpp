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
    StdoutWriter() : buffer_(new char[1024 * 60]), writen_(0) {}
    ~StdoutWriter() { delete[] buffer_; }
    void append(const char* data, int32_t len) override
    {
        memcpy(buffer_ + writen_, data, len);
        writen_ += len;
    }
    void flush() override { std::cout << writen_ << " ========================" << std::endl; }
    uint32_t writtenBytes() const override { return writen_; }
    void reset() override
    {
        delete[] buffer_;
        buffer_ = new char[1024 * 60];
        writen_ = 0;
    }

private:
    char* buffer_;
    uint32_t writen_;
};
}  // namespace phase0