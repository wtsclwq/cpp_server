/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-17 22:35:23
 * @LastEditTime: 2023-03-17 23:44:09
 */
#include "../src/include/log/log_manager.h"
#include "../src/include/util/macro.h"
#include "../src/include/util/thread_util.h"
auto main() -> int {
    LOG_INFO(ROOT_LOGGER, wtsclwq::BacktraceToString(10, 0, "   "));
    WTSCLWQ_ASSERT(1 > 3, "aa");
}