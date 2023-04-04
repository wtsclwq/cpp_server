//
// Created by wtsclwq on 23-4-4.
//

#include "../include/serialize/stream.h"

namespace wtsclwq {
auto Stream::ReadFixSizeToBuf(void *buffer, size_t length) -> int {
    size_t offset = 0;
    int64_t left = length;
    while (left > 0) {
        // 不断尝试读取left长度的数据，每次读取到len长度的数据，就将需求减少len
        int64_t len = ReadToBuf(static_cast<char *>(buffer) + offset, left);
        if (len <= 0) {
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;
}

auto Stream::ReadFixSizeToArray(ByteArray::ptr ba, size_t length) -> int {
    int64_t left = length;
    while (left > 0) {
        // 不断尝试读取left长度的数据，每次读取到len长度的数据，就将需求减少len
        int64_t len = ReadToArray(ba, left);
        if (len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}

auto Stream::WriteFixSizeFromBuf(const void *buffer, size_t length) -> int {
    size_t offset = 0;
    int64_t left = length;
    while (left > 0) {
        // 不断尝试写入left长度的数据，每次写入len长度的数据，就将需求减少len
        int64_t len =
            WriteFromBuf(static_cast<const char *>(buffer) + offset, left);
        if (len <= 0) {
            return len;
        }
        offset += len;
        left -= len;
    }
    return length;
}

auto Stream::WriteFixSizeFromArray(ByteArray::ptr ba, size_t length) -> int {
    int64_t left = length;
    while (left > 0) {
        // 不断尝试写入left长度的数据，每次写入len长度的数据，就将需求减少len
        int64_t len = WriteFromArray(ba, left);
        if (len <= 0) {
            return len;
        }
        left -= len;
    }
    return length;
}
}  // namespace wtsclwq