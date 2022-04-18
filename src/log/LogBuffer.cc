#include "LogBuffer.h"

#include <string.h>

#include <cstddef>
#include <iostream>

namespace phase0
{
LogBuffer::LogBuffer(size_t size) : size_(size), cur_(0), available_(size) { data_ = new char[size_]{'\0'}; }

LogBuffer::~LogBuffer() { delete[] data_; }

void LogBuffer::append(const char* data, size_t length)
{
    memcpy(data_ + cur_, data, length);
    cur_ += length;
    available_ -= length;
}

void LogBuffer::clear()
{
    cur_ = 0;
    available_ = size_;
}

const char* LogBuffer::data() const { return data_; }

size_t LogBuffer::available() const { return available_; }

size_t LogBuffer::length() const { return cur_; }
}  // namespace phase0