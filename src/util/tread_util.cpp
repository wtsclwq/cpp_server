/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-17 22:29:33
 * @LastEditTime: 2023-03-21 18:13:30
 */
#include <execinfo.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <cstdlib>
#include <sstream>
#include <vector>

#include "../include/concurrency/fiber.h"
#include "../include/concurrency/thread.h"
#include "../include/log/log_manager.h"
#include "../include/util/thread_util.h"

namespace wtsclwq {

static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");  // NOLINT

auto GetThreadName() -> std::string { return Thread::GetCurThreadName(); }

auto GetThreadId() -> pid_t { return static_cast<pid_t>(syscall(SYS_gettid)); }

auto GetFiberId() -> uint64_t { return Fiber::GetCurFiberId(); }

void Backtrace(std::vector<std::string> &back_trace, int size, int skip) {
    size_t mem_size = sizeof(void *) * size;
    // 保存调用栈中各个符号的地址
    // void **array = (void **)(std::malloc(mem_size));
    void **array = new void *[mem_size];

    // 获得的调用栈层数
    int layers = ::backtrace(array, size);
    // 根据地址拿到符号字符串数组
    char **symbols = backtrace_symbols(array, layers);
    if (symbols == nullptr) {
        LOG_ERROR(sys_logger, "backtrace_synbols error");
        return;
    }
    for (size_t i = skip; i < layers; ++i) {
        back_trace.emplace_back(symbols[i]);  // NOLINT
    }

    delete[] array;
    free(symbols);  // NOLINT
}

auto BacktraceToString(int size, int skip, const std::string &prefix)
    -> std::string {
    std::vector<std::string> back_trace;
    Backtrace(back_trace, size, skip);
    std::stringstream sstream;
    for (const auto &symbol : back_trace) {
        sstream << prefix << symbol << "\n";
    }
    return sstream.str();
}

}  // namespace wtsclwq