/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-19 16:31:08
 */
#pragma once

#include <execinfo.h>

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace wtsclwq {

auto GetThreadName() -> std::string;
auto GetThreadId() -> uint64_t;
auto GetFiberId() -> uint64_t;
void Backtrace(std::vector<std::string>* back_trace, int size, int skip);
auto BacktraceToString(int size, int skip = 2, const std::string& prefix = "")
    -> std::string;
}  // namespace wtsclwq