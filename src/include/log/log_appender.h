/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-21 14:13:11
 */
#pragma once

#include <bits/types/FILE.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "log_event.h"
#include "log_formatter.h"
#include "log_level.h"
namespace wtsclwq {
/**
 * 日志输出目的地抽象类
 */
class LogAppender {
  public:
    LogAppender(const LogAppender&) = delete;
    LogAppender(LogAppender&&) = delete;
    auto operator=(const LogAppender&) -> LogAppender& = delete;
    auto operator=(LogAppender&&) -> LogAppender& = delete;

    using ptr = std::shared_ptr<LogAppender>;
    using MutexType = std::mutex;

    explicit LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    virtual ~LogAppender() = default;
    virtual void Log(const LogEvent::ptr& log_event) = 0;
    [[nodiscard]] auto GetFormatter() const -> LogFormatter::ptr;
    void SetFormatter(LogFormatter::ptr formatter);
    [[nodiscard]] auto GetLevel() const -> LogLevel::Level;
    auto SetLevel(LogLevel::Level level);
    auto GetMutex() -> MutexType&;

  private:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
    mutable MutexType m_mutex;  // GetXX方法是const的
};

/* 标准输出日志输出目的地 */
class StdoutLogAppender : public LogAppender {
  public:
    using ptr = std::shared_ptr<StdoutLogAppender>;

    explicit StdoutLogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    void Log(const LogEvent::ptr& log_event) override;
};

/* 文件日志输出目的地 */
class FileLogAppender : public LogAppender {
  public:
    using ptr = std::shared_ptr<FileLogAppender>;

    explicit FileLogAppender(std::string file_name,
                             LogLevel::Level level = LogLevel::Level::DEBUG);
    void Log(const LogEvent::ptr& log_event) override;
    auto ReOpen() -> bool;

  private:
    std::string m_file_name;
    std::ofstream m_file_stream;
};
}  // namespace wtsclwq
