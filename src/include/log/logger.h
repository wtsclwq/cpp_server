#pragma once

#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>

#include "../util/thread_util.h"
#include "log_appender.h"
#include "log_event.h"
#include "log_formatter.h"

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

/**
 * 日志器
 */
class Logger {
   public:
    using ptr = std::shared_ptr<Logger>;

    Logger();
    Logger(std::string name, LogLevel::Level level, std::string pattern);

    /* 根据日志事件打印日志内容 */
    void Log(const LogEvent::ptr& log_event);

    /* 为日志对象添加一个日志输出目的地 */
    void AddAppender(const LogAppender::ptr& log_appender);
    /* 为日志对象删除某个日志输出目的地 */
    void DelAppender(const LogAppender::ptr& log_appender);

    auto GetLevel() const -> LogLevel::Level { return m_level; }
    void SetLevel(LogLevel::Level level) { m_level = level; }

   private:
    std::string m_name;                           // 日志名称
    LogLevel::Level m_level;                      // 日志级别
    std::string m_pattern;                        // 默认格式化模板
    LogFormatter::ptr m_formatter;                // 默认格式化器
    std::list<LogAppender::ptr> m_log_appenders;  // Appender集合
};
}  // namespace wtsclwq
