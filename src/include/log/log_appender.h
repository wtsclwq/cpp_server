#pragma once

#include <fstream>
#include <iostream>
#include <memory>

#include "log_event.h"
#include "log_formatter.h"
#include "log_level.h"

namespace wtsclwq {

/**
 * 日志输出目的地抽象类
 */
class LogAppender {
   public:
    using ptr = std::shared_ptr<LogAppender>;

    explicit LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    LogAppender(const LogAppender&) = default;
    LogAppender(LogAppender&&) = delete;
    auto operator=(const LogAppender&) -> LogAppender& = default;
    auto operator=(LogAppender&&) -> LogAppender& = delete;
    virtual ~LogAppender() = default;
    virtual void Log(const LogEvent::ptr& log_event) = 0;
    auto GetFormatter() const -> LogFormatter::ptr { return m_formatter; }
    void SetFormatter(LogFormatter::ptr formatter) { m_formatter = std::move(formatter); }
    auto GetLevel() const -> LogLevel::Level { return m_level; }
    auto SetLevel(LogLevel::Level level) { m_level = level; }

   private:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
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
