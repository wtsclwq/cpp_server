/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-21 14:13:06
 */
#pragma once

#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <mutex>
#include <vector>

#include "log_appender.h"
#include "log_event.h"
#include "log_formatter.h"
#include "log_level.h"

/**
 * 日志器
 */
namespace wtsclwq {

class Logger {
  public:
    using ptr = std::shared_ptr<Logger>;
    using MutexType = std::mutex;
    Logger();
    Logger(std::string name, LogLevel::Level level, std::string pattern);

    /* 根据日志事件打印日志内容 */
    void Log(const LogEvent::ptr& log_event);

    /* 为日志对象添加一个日志输出目的地 */
    void AddAppender(LogAppender::ptr&& log_appender);

    /* 为日志对象删除某个日志输出目的地 */
    void DelAppender(const LogAppender::ptr& log_appender);

    [[nodiscard]] auto GetName() const -> std::string;
    [[nodiscard]] auto GetLevel() const -> LogLevel::Level;
    void SetLevel(LogLevel::Level level) { m_level = level; }

  private:
    std::string m_name;                           // 日志名称
    LogLevel::Level m_level;                      // 日志级别
    std::string m_pattern;                        // 默认格式化模板
    LogFormatter::ptr m_formatter;                // 默认格式化器
    std::list<LogAppender::ptr> m_log_appenders;  // Appender集合
    MutexType m_mutex;
};
}  // namespace wtsclwq
