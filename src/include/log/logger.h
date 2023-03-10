/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-12 15:12:43
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <vector>

#include "../util/thread_util.h"
#include "log_appender.h"
#include "log_event.h"
#include "log_formatter.h"
#include "log_level.h"

#define MAKE_LOG_EVENT(level, content)                                                 \
    std::make_shared<wtsclwq::LogEvent>(__FILE__, __LINE__, content,                   \
                                        wtsclwq::GetThreadId(), wtsclwq::GetFiberId(), \
                                        time(nullptr), level)

#define LOG_BY_LEVEL(logger, level, message) logger->Log(MAKE_LOG_EVENT(level, message))

#define LOG_DEBUG(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::DEBUG, message)

#define LOG_INFO(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::INFO, message)

#define LOG_WARN(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::WARN, message)

#define LOG_ERROR(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::ERROR, message)

#define LOG_FATAL(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::FATAL, message)

#define LOG_CUSTOM_LEVEL(logger, level, pattern, ...)                 \
    {                                                                 \
        char* buffer = new char;                                      \
        int length = asprintf(&buffer, pattern, ##__VA_ARGS__);       \
        if (length != -1) {                                           \
            LOG_BY_LEVEL(logger, level, std::string(buffer, length)); \
            delete buffer;                                            \
        }                                                             \
    }

#define LOG_CUSTOM_DEBUG(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::DEBUG, pattern, argv)

#define LOG_CUSTOM_INFO(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::INFO, pattern, argv)

#define LOG_CUSTOM_WARN(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::WARN, pattern, argv)

#define LOG_CUSTOM_ERROR(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::ERROR, pattern, argv)

#define LOG_CUSTOM_FATAL(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::FATAL, pattern, argv)

namespace wtsclwq {

struct LoggerConfig {
    std::string name;                                 // ???????????????
    LogLevel::Level level{LogLevel::Level::UNKNOWN};  // ??????????????????
    std::string pattern;                              // ????????????
    std::vector<LogAppenderConfig> appenders;         // ???????????????

    auto operator==(const LoggerConfig& other) const -> bool {
        return name == other.name;
    }
};

/**
 * ?????????
 */
class Logger {
   public:
    using ptr = std::shared_ptr<Logger>;

    Logger();
    Logger(std::string name, LogLevel::Level level, std::string pattern);

    /* ???????????????????????????????????? */
    void Log(const LogEvent::ptr& log_event);

    /* ???????????????????????????????????????????????? */
    void AddAppender(LogAppender::ptr&& log_appender);
    /* ???????????????????????????????????????????????? */
    void DelAppender(const LogAppender::ptr& log_appender);

    auto GetName() const -> std::string { return m_name; }
    auto GetLevel() const -> LogLevel::Level { return m_level; }
    void SetLevel(LogLevel::Level level) { m_level = level; }

   private:
    std::string m_name;                           // ????????????
    LogLevel::Level m_level;                      // ????????????
    std::string m_pattern;                        // ?????????????????????
    LogFormatter::ptr m_formatter;                // ??????????????????
    std::list<LogAppender::ptr> m_log_appenders;  // Appender??????
};
}  // namespace wtsclwq
