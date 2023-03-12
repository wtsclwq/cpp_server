/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-11 23:59:34
 */
#pragma once

#include <bits/types/FILE.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "log_event.h"
#include "log_formatter.h"
#include "log_level.h"

namespace wtsclwq {

struct LogAppenderConfig {
    enum Type { STDOUT = 0, FILE = 1 };
    LogAppenderConfig::Type type{Type::STDOUT};       // 目的地的类型
    LogLevel::Level level{LogLevel::Level::UNKNOWN};  // 日志等级
    std::string pattern;                              // 目的地的日志格式
    std::string file;  // 目的地的目标文件path,仅在FILE时有效

    auto operator==(const LogAppenderConfig& other) const -> bool {
        return type == other.type && level == other.level && pattern == other.pattern &&
               file == other.file;
    }
};

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
