#pragma once

#define PHASE0_LITTLE_ENDIAN 1
#define PHASE0_BIG_ENDIAN 2

#include <byteswap.h>
#include <stdint.h>

#include <type_traits>

namespace phase0
{
template <typename T>
concept ByteSize8 = sizeof(T) == sizeof(uint64_t);

template <typename T>
concept ByteSize4 = sizeof(T) == sizeof(uint32_t);

template <typename T>
concept ByteSize2 = sizeof(T) == sizeof(uint16_t);

template <ByteSize8 T>
T byteswap(T value) { return (T)bswap_64((uint64_t)value); }

template <ByteSize4 T>
T byteswap(T value) { return (T)bswap_32((uint32_t)value); }

template <ByteSize2 T>
T byteswap(T value) { return (T)bswap_16((uint16_t)value); }

#if BYTE_ORDER == BIG_ENDIAN
#define PHASE0_BYTE_ORDER PHASE0_BIG_ENDIAN
#else
#define PHASE0_BYTE_ORDER PHASE0_LITTLE_ENDIAN
#endif

#if PHASE0_BYTE_ORDER == PHASE0_BIG_ENDIAN

template <class T>
T byteswapOnLittleEndian(T t)
{
    return t;
}

template <class T>
T byteswapOnBigEndian(T t)
{
    return byteswap(t);
}

#else

template <class T>
T byteswapOnLittleEndian(T t)
{
    return byteswap(t);
}

template <class T>
T byteswapOnBigEndian(T t)
{
    return t;
}
#endif

}  // namespace phase0