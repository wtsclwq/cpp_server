/*
 * @Description:
 * @LastEditTime: 2023-03-23 23:23:58
 */
#include "../src/include/concurrency/scheduler.h"

#include <unistd.h>

#include "../src/include/log/log_manager.h"

auto logger = ROOT_LOGGER;

auto main() -> int {
    LOG_CUSTOM_DEBUG(logger, "主线程 %d 测试开始", wtsclwq::GetThreadId());
    wtsclwq::Scheduler scheduler(5, true, "aaaa");
    scheduler.Start();
    scheduler.Stop();
    LOG_CUSTOM_DEBUG(logger, "主线程 %d 测试结束", wtsclwq::GetThreadId());
}