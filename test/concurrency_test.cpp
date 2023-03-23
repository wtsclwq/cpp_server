/*
 * @Description:
 * @LastEditTime: 2023-03-23 20:37:02
 */
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/include/concurrency/fiber.h"
#include "../src/include/concurrency/lock.h"
#include "../src/include/concurrency/thread.h"
#include "../src/include/log/log_manager.h"
#include "yaml-cpp/node/parse.h"

auto logger = GET_LOGGER_BY_NAME("system");
void run_in_fiber() {
    LOG_CUSTOM_INFO(logger, "thread: %s run_in_fiber 111...",
                    wtsclwq::GetThreadName().c_str());

    wtsclwq::Fiber::YieldToHold();
    LOG_CUSTOM_INFO(logger, "thread: %s run_in_fiber 222...",
                    wtsclwq::GetThreadName().c_str());

    wtsclwq::Fiber::YieldToHold();
    LOG_CUSTOM_INFO(logger, "thread: %s run_in_fiber 333...",
                    wtsclwq::GetThreadName().c_str());

    wtsclwq::Fiber::YieldToHold();
    LOG_CUSTOM_INFO(logger, "thread: %s run_in_fiber 444...",
                    wtsclwq::GetThreadName().c_str());
}

void test_fiber() {
    wtsclwq::Fiber::GetCurFiber();
    wtsclwq::Fiber::ptr fiber(new wtsclwq::Fiber(run_in_fiber));

    LOG_CUSTOM_INFO(logger, "thread: %s swap [in] ro run_in_fiber 1",
                    wtsclwq::GetThreadName().c_str());
    fiber->SwapIn();
    LOG_CUSTOM_INFO(logger, "thread: %s swap [out] from run_in_fiber 1",
                    wtsclwq::GetThreadName().c_str());

    LOG_CUSTOM_INFO(logger, "thread: %s swap [in] to run_in_fiber 2",
                    wtsclwq::GetThreadName().c_str());
    fiber->SwapIn();
    LOG_CUSTOM_INFO(logger, "thread: %s swap [out] from run_in_fiber 2",
                    wtsclwq::GetThreadName().c_str());

    LOG_CUSTOM_INFO(logger, "thread: %s swap [in] to run_in_fiber 3",
                    wtsclwq::GetThreadName().c_str());
    fiber->SwapIn();
    LOG_CUSTOM_INFO(logger, "thread: %s swap [out] from run_in_fiber 3",
                    wtsclwq::GetThreadName().c_str());

    LOG_CUSTOM_INFO(logger, "thread: %s swap [in] to run_in_fiber 4",
                    wtsclwq::GetThreadName().c_str());
    fiber->SwapIn();
    LOG_CUSTOM_INFO(logger, "thread: %s swap [out] from run_in_fiber 4",
                    wtsclwq::GetThreadName().c_str());
}

int count = 0;
wtsclwq::RWLock rw_lock;
void fun1() {
    std::stringstream sstream;
    sstream << "\nname: " << wtsclwq::Thread::GetCurThreadName() << "\n"
            << "cur.name: " << wtsclwq::Thread::GetCurThread()->GetName()
            << "\n"
            << "id: " << wtsclwq::GetThreadId() << "\n"
            << "cur.id: " << wtsclwq::Thread::GetCurThread()->GetId() << "\n";
    LOG_INFO(logger, sstream.str());

    for (int i = 0; i < 100000; ++i) {
        wtsclwq::ScopedWriteLock w_lock(rw_lock);
        ++count;
    }
}

void test_thread() {
    LOG_CUSTOM_INFO(logger, "thread: %s test begin",
                    wtsclwq::GetThreadName().c_str());

    std::vector<wtsclwq::Thread::ptr> thrs;
    for (int i = 0; i != 3; ++i) {
        wtsclwq::Thread::ptr thr(
            new wtsclwq::Thread(&test_fiber, "name_" + std::to_string(i)));
        thrs.push_back(thr);
    }

    for (int i = 0; i != 3; ++i) {
        thrs[i]->Join();
    }

    LOG_CUSTOM_INFO(logger, "thread: %s test end",
                    wtsclwq::GetThreadName().c_str());
}

void log1() {
    int i = 1;
    while (i > 0) {
        LOG_INFO(logger, "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        --i;
    }
}

void log3() {
    int i = 1;
    while (i > 0) {
        LOG_INFO(logger, "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb");
        --i;
    }
}

void test_concurrency_log() {
    YAML::Node node = YAML::LoadFile("/home/wtsclwq/desktop/log_config.yml");
    wtsclwq::Config::LoadFromYaml(node);

    logger = GET_LOGGER_BY_NAME("system");
    LOG_INFO(logger, "testbegin");
    std::vector<wtsclwq::Thread::ptr> thrs;

    for (int i = 0; i < 5; ++i) {  // 5*100000
        wtsclwq::Thread::ptr thr1(
            new wtsclwq::Thread(&log1, "name_" + std::to_string(i * 2)));
        thrs.push_back(thr1);
        wtsclwq::Thread::ptr thr2(
            new wtsclwq::Thread(&log3, "name_" + std::to_string(i * 2 + 1)));
        thrs.push_back(thr2);
    }

    for (auto& thr : thrs) {
        thr->Join();
    }
    LOG_INFO(logger, "teseend");
}
auto main() -> int {
    test_fiber();
    return 0;
}