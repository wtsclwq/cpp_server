/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-19 23:22:04
 */
#include <memory>

#include "../src/include/log/log_appender.h"
#include "../src/include/log/log_event.h"
#include "../src/include/log/log_formatter.h"
#include "../src/include/log/log_level.h"
#include "../src/include/log/log_manager.h"

auto main() -> int {
    auto logger =
        wtsclwq::SingltonLogManager::GetInstancePtr()->GetGlobalLogger();
    // auto logger = std::make_shared<wtsclwq::Logger>();
    wtsclwq::LogEvent log_event{"lwq", 1, "wtsclwq", "default", 2, 3, 1};

    auto appender_ptr = std::make_shared<wtsclwq::FileLogAppender>(
        "/home/wtsclwq/desktop/log.txt", wtsclwq::LogLevel::Level::DEBUG);

    logger->AddAppender(appender_ptr);

    LOG_CUSTOM_DEBUG(logger, "%d%d", 1000, 2000);

    return 0;
}