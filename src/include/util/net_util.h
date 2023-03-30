//
// Created by wtsclwq on 23-3-30.
//

#pragma once

#include <byteswap.h>

#include <cstdint>

namespace wtsclwq {

template <class T>
static auto CreateMask(uint32_t bits) -> T {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}
template <class T>
static auto CountBytes(T value) -> uint32_t {
    uint32_t result = 0;
    for (; value; ++result) {
        value &= value - 1;
    }
    return result;
}

/**
 * @brief 8字节类型的字节序转化
 */
template <class T>
auto ByteSwap(T value) ->
    typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type {
    return static_cast<T>(bswap_64(static_cast<uint64_t>(value)));
}

/**
 * @brief 4字节类型的字节序转化
 */
template <class T>
auto ByteSwap(T value) ->
    typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type {
    return static_cast<T>(bswap_32(static_cast<uint32_t>(value)));
}

/**
 * @brief 2字节类型的字节序转化
 */
template <class T>
auto ByteSwap(T value) ->
    typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type {
    return static_cast<T>(bswap_16(static_cast<uint16_t>(value)));
}

/**
 * @brief 将数据转换为大端字节序，只在小端机器上执行byteswap, 在大端机器上什么都不做
 */
template <class T>
auto GetBigEndianValue(T t) -> T {
    if (BYTE_ORDER == BIG_ENDIAN) {  // NOLINT
        return t;                    // NOLINT
    }
    return ByteSwap(t);
}

/**
 * @brief 将数据转换为小端字节序，只在大端机器上执行byteswap, 在小端机器上什么都不做
 */
template <class T>
auto GetLittleEndianValue(T t) -> T {
    if (BYTE_ORDER == BIG_ENDIAN) {  // NOLINT
        return ByteSwap(t);          // NOLINT
    }
    return t;
}
}  // namespace wtsclwq
