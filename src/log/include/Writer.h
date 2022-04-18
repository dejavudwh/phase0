#pragma once

#include <cstdint>
#include <memory>

namespace phase0
{
class Writer
{
public:
    using ptr = std::shared_ptr<Writer>;

    Writer(){};
    virtual ~Writer(){};
    virtual void append(const char* data, int32_t len) = 0;
    virtual void flush() = 0;
    virtual uint32_t writtenBytes() const = 0;
    virtual void reset() = 0;
};
}  // namespace phase0