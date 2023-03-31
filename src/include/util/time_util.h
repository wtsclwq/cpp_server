/*
 * @Description:
 * @LastEditTime: 2023-03-26 20:25:36
 */
#pragma once
#include <bits/types/struct_timeval.h>
#include <sys/time.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
static const int BASE_NUMBER_OF_SECONDS = 1000;

namespace wtsclwq {
auto inline GetCurrentMS() -> uint64_t {
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = now_ms.time_since_epoch();
    return static_cast<uint64_t>(value.count());
}

auto inline GetCurrentUS() -> uint64_t {
    auto now = std::chrono::system_clock::now();
    auto now_us = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    auto value = now_us.time_since_epoch();
    return static_cast<uint64_t>(value.count());
}
}  // namespace wtsclwq