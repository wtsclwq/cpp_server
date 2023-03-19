/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-19 16:29:53
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <vector>

#include "log_appender.h"
#include "log_event.h"
#include "log_formatter.h"
#include "log_level.h"

namespace wtsclwq {

struct LoggerConfig {
    std::string name;                                 // 日志器名称
    LogLevel::Level level{LogLevel::Level::UNKNOWN};  // 日志有效等级
    std::string pattern;                              // 日志格式
    std::vector<LogAppenderConfig> appenders;         // 目的地集合

    auto operator==(const LoggerConfig& other) const -> bool {
        return name == other.name;
    }
};

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
    void AddAppender(LogAppender::ptr&& log_appender);
    /* 为日志对象删除某个日志输出目的地 */
    void DelAppender(const LogAppender::ptr& log_appender);

    auto GetName() const -> std::string { return m_name; }
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
