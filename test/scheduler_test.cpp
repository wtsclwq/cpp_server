/*
 * @Description:
 * @LastEditTime: 2023-03-24 16:02:57
 */
#include "../src/include/concurrency/scheduler.h"

#include <unistd.h>

#include "../src/include/log/log_manager.h"

auto logger = ROOT_LOGGER;

void fun() {
    static int s_count = 5;
    LOG_CUSTOM_INFO(logger, "fun1... s_count = %d", s_count);
    sleep(1);
    if (--s_count >= 0) {
        // wtsclwq::Scheduler::GetThisThreadScheduler()->Schedule(fun);
        wtsclwq::Scheduler::GetThisThreadScheduler()->Schedule(
            fun, wtsclwq::GetThreadId());
    }
}

auto main() -> int {
    LOG_CUSTOM_DEBUG(logger, "主线程 %d 测试开始", wtsclwq::GetThreadId());
    wtsclwq::Scheduler scheduler(3, false, "aaaa");
    scheduler.Start();
    scheduler.Schedule(fun);
    scheduler.Stop();
    LOG_CUSTOM_DEBUG(logger, "主线程 %d 测试结束", wtsclwq::GetThreadId());
}