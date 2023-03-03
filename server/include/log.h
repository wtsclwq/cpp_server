//
// Created by wtsclwq on 23-3-2.
//

#ifndef SYLAR_LEARN_LOG_H
#define SYLAR_LEARN_LOG_H

#include <stdint.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace wtsclwq {

/**
 * 日志级别
 */
class LogLevel {
   public:
    enum Level { DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5 };
    static auto ToString(LogLevel::Level level) -> std::string;
};

/**
 *日志事件
 */
class LogEvent {
   public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(const std::string& file_name, uint32_t line, uint32_t thread_id,
             uint32_t fiber_id, time_t time, const std::string& content,
             LogLevel::Level level = LogLevel::Level::DEBUG)
        : m_file_name(file_name), m_line(line), m_thread_id(thread_id),
          m_fiber_id(fiber_id), m_time(time), m_content(content),
          m_level(level) {}

    const std::string& GetFileName() const { return m_file_name; }
    LogLevel::Level GetLevel() const { return m_level; }
    uint32_t GetLine() const { return m_line; }
    uint32_t GetThreadId() const { return m_thread_id; }
    uint32_t GetFiberId() const { return m_fiber_id; }
    time_t GetTime() const { return m_time; }
    const std::string& GetContent() { return m_content; }

    void SetLevel(LogLevel::Level level) { m_level = level; }

   private:
    LogLevel::Level m_level;  // 日志等级
    std::string m_file_name;  // 文件名
    uint32_t m_line;          // 行号
    uint32_t m_elapse{0};     // 程序启动开始到现在的毫秒数
    uint32_t m_thread_id;     // 线程id
    uint32_t m_fiber_id;      // 携程id
    time_t m_time;            // 报告时间
    std::string m_content{};  // 日志内容
};

/**
 * 日志格式器
 */
class LogFormatter {
   public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string& pattern);
    auto Format(LogEvent::ptr log_event) -> std::string;
    void Init();

    class FormatItem {
       public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() = default;
        virtual void Format(std::ostringstream& os, LogEvent::ptr event) = 0;
    };

   private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

class PlainFormatItem : public LogFormatter::FormatItem {
   public:
    explicit PlainFormatItem(const std::string& str) : m_str(str) {}
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << m_str;
    }

   private:
    std::string m_str;
};

class LevelFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << LogLevel::ToString(ev->GetLevel());
    }
};

class FilenameFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << ev->GetFileName();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << ev->GetLine();
    }
};

class ThreadIDFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << ev->GetThreadId();
    }
};

class FiberIDFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << ev->GetFiberId();
    }
};

class TimeFormatItem : public LogFormatter::FormatItem {
   public:
    explicit TimeFormatItem(const std::string& str = "%Y-%m-%d %H:%M:%S")
        : m_time_pattern(str) {
        if (m_time_pattern.empty()) {
            m_time_pattern = "%Y-%m-%d %H:%M:%S";
        }
    }
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        struct tm time_struct {};
        time_t time_l = ev->GetTime();
        localtime_r(&time_l, &time_struct);
        char buffer[64]{0};
        strftime(buffer, sizeof(buffer), m_time_pattern.c_str(), &time_struct);
        out << buffer;
    }

   private:
    std::string m_time_pattern;
};

class ContentFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << ev->GetContent();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << std::endl;
    }
};

class PercentSignFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << '%';
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
   public:
    void Format(std::ostringstream& out, LogEvent::ptr ev) override {
        out << '\t';
    }
};

/**
 * 日志输出地
 */
class LogAppender {
   public:
    typedef std::shared_ptr<LogAppender> ptr;

    LogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    virtual ~LogAppender() = default;
    virtual void Log(LogLevel::Level level, LogEvent::ptr log_event) = 0;
    auto GetFormatter() -> LogFormatter::ptr { return m_formatter; }
    void SetFormatter(LogFormatter::ptr formatter) {
        m_formatter = std::move(formatter);
    }

   protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

class StdoutLogAppender : public LogAppender {
   public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
    StdoutLogAppender(LogLevel::Level level = LogLevel::Level::DEBUG);
    virtual void Log(LogLevel::Level level, LogEvent::ptr log_event) override;
};

class FileLogAppender : public LogAppender {
   public:
    typedef std::shared_ptr<FileLogAppender> ptr;
    FileLogAppender(const std::string& file_name,
                    LogLevel::Level level = LogLevel::Level::DEBUG);
    virtual void Log(LogLevel::Level level, LogEvent::ptr log_event) override;
    auto ReOpen() -> bool;

   private:
    std::string m_file_name;
    std::ofstream m_file_stream;
};
/**
 * 日志器
 */
class Logger {
   public:
    typedef std::shared_ptr<Logger> ptr;

    Logger();
    Logger(const std::string name, LogLevel::Level level,
           const std::string& pattern);

    void Log(const LogEvent::ptr log_event);
    void Debug(const LogEvent::ptr log_event);
    void InFo(const LogEvent::ptr log_event);
    void Warn(const LogEvent::ptr log_event);
    void Error(const LogEvent::ptr log_event);
    void Fatal(const LogEvent::ptr log_event);

    void AddAppender(LogAppender::ptr log_appender);
    void DelAppender(LogAppender::ptr log_appender);

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

#endif  // SYLAR_LEARN_LOG_H
