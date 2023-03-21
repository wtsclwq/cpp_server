#pragma once

#include <array>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <vector>

#include "../concurrency/lock.h"
#include "log_event.h"
#include "log_level.h"
/**
 * 日志格式器
 */
namespace wtsclwq {

class LogFormatter {
  public:
    using ptr = std::shared_ptr<LogFormatter>;
    using MutexType = std::mutex;
    explicit LogFormatter(std::string pattern);
    auto Format(const LogEvent::ptr& log_event) -> std::string;
    void Init();

    class FormatItem {
      public:
        using ptr = std::shared_ptr<FormatItem>;

        FormatItem() = default;
        FormatItem(const FormatItem&) = default;
        FormatItem(FormatItem&&) = delete;
        auto operator=(const FormatItem&) -> FormatItem& = default;
        auto operator=(FormatItem&&) -> FormatItem& = delete;
        virtual ~FormatItem() = default;
        virtual void Format(std::ostringstream& oss,
                            const LogEvent::ptr& log_event) = 0;
    };

  private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
};

class PlainFormatItem : public LogFormatter::FormatItem {
  public:
    explicit PlainFormatItem(std::string str) : m_str(std::move(str)) {}
    void Format(std::ostringstream& out,
                const LogEvent::ptr& /*event*/) override {
        out << m_str;
    }

  private:
    std::string m_str;
};

class LevelFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        out << LogLevel::ToString(event->GetLevel());
    }
};

class FilenameFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        out << event->GetFileName();
    }
};

class LineFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        out << event->GetLine();
    }
};

class ThreadNameFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        out << event->GetThreadName();
    }
};

class ThreadIDFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        out << event->GetThreadId();
    }
};

class FiberIDFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        out << event->GetFiberId();
    }
};

class TimeFormatItem : public LogFormatter::FormatItem {
  public:
    explicit TimeFormatItem(std::string str = "%Y-%m-%d %H:%M:%S")
        : m_time_pattern(std::move(str)) {
        if (m_time_pattern.empty()) {
            m_time_pattern = "%Y-%m-%d %H:%M:%S";
        }
    }

    void Format(std::ostringstream& out, const LogEvent::ptr& event) override {
        struct tm time_struct {};
        time_t time_l = event->GetTime();
        localtime_r(&time_l, &time_struct);

        out << std::put_time(&time_struct, m_time_pattern.c_str());
    }

  private:
    std::string m_time_pattern;
};

class ContentFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out, const LogEvent::ptr& evnet) override {
        out << evnet->GetContent();
    }
};

class NewLineFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out,
                const LogEvent::ptr& /*evnet*/) override {
        out << std::endl;
    }
};

class PercentSignFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out,
                const LogEvent::ptr& /*evnet*/) override {
        out << '%';
    }
};

class TabFormatItem : public LogFormatter::FormatItem {
  public:
    void Format(std::ostringstream& out,
                const LogEvent::ptr& /*event*/) override {
        out << '\t';
    }
};
}  // namespace wtsclwq
