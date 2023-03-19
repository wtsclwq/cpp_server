/*
 * @Description:
 * @LastEditTime: 2023-03-19 15:35:04
 */
#include "../src/include/concurrency/fiber.h"
#include "../src/include/log/log_manager.h"

auto logger = ROOT_LOGGER;

void run_in_fiber() {
    LOG_INFO(logger, "run_in_fiber 111...");
    wtsclwq::Fiber::YieldToHold();
    LOG_INFO(logger, "run_in_fiber 222...");
    wtsclwq::Fiber::YieldToHold();
    LOG_INFO(logger, "run_in_fiber 333...");
    wtsclwq::Fiber::YieldToHold();
    LOG_INFO(logger, "run_in_fiber 444...");
}

void test_fiber() {
    wtsclwq::Fiber::GetCurFiber();
    wtsclwq::Fiber::ptr fiber(new wtsclwq::Fiber(run_in_fiber));
    
    LOG_INFO(logger, "swap [in] ro run_in_fiber 1");
    fiber->SwapIn();
    LOG_INFO(logger, "swap [out] from run_in_fiber 1");

    LOG_INFO(logger, "swap [in] to run_in_fiber 2");
    fiber->SwapIn();
    LOG_INFO(logger, "swap [out] from run_in_fiber 2");

    LOG_INFO(logger, "swap [in] to run_in_fiber 3");
    fiber->SwapIn();
    LOG_INFO(logger, "swap [out] from run_in_fiber 3");

    LOG_INFO(logger, "swap [in] to run_in_fiber 4");
    fiber->SwapIn();
    LOG_INFO(logger, "swap [out] from run_in_fiber 4");
}

auto main() -> int {
    LOG_INFO(logger, "main 111");
    test_fiber();
    LOG_INFO(logger, "main 222");
    return 0;
}