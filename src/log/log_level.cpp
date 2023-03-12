/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-12 14:20:32
 */

#include "../include/log/log_level.h"

namespace wtsclwq {

auto LogLevel::ToString(LogLevel::Level level) -> std::string {
    std::string result;
    switch (level) {
        case DEBUG:
            result = "DEBUG";
            break;
        case INFO:
            result = "INFO";
            break;
        case WARN:
            result = "WARN";
            break;
        case ERROR:
            result = "ERROR";
            break;
        case FATAL:
            result = "FATAL";
            break;
        case UNKNOWN:
            result = "UNKNOWN";
            break;
    }
    return result;
}

auto LogLevel::FromString(const std::string &level_str) -> LogLevel::Level {
    if (level_str == "DEBUG" || level_str == "debug") {
        return Level::DEBUG;
    }
    if (level_str == "INFO" || level_str == "info") {
        return Level::INFO;
    }
    if (level_str == "WARN" || level_str == "warn") {
        return Level::WARN;
    }
    if (level_str == "ERROR" || level_str == "error") {
        return Level::ERROR;
    }
    if (level_str == "FATAL" || level_str == "fatal") {
        return Level::FATAL;
    }
    return Level::UNKNOWN;
}
}  // namespace wtsclwq
