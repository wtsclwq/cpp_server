/*
 * @Description:
 * @LastEditTime: 2023-03-21 18:04:13
 */
#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "log_level.h"
namespace wtsclwq {

/**
 *日志事件
 */
class LogEvent {
  public:
    using ptr = std::shared_ptr<LogEvent>;

    LogEvent(std::string file_name, uint32_t line, std::string content,
             std::string thread_name, uint32_t thread_id, uint32_t fiber_id,
             time_t time, LogLevel::Level level = LogLevel::Level::DEBUG)
        : m_file_name(std::move(file_name)), m_line(line),
          m_thread_name(std::move(thread_name)), m_thread_id(thread_id),
          m_fiber_id(fiber_id), m_time(time), m_content(std::move(content)),
          m_level(level) {}

    [[nodiscard]] auto GetFileName() const -> std::string {
        return m_file_name;
    }
    [[nodiscard]] auto GetLevel() const -> LogLevel::Level { return m_level; }
    [[nodiscard]] auto GetLine() const -> uint32_t { return m_line; }
    [[nodiscard]] auto GetThreadName() const -> std::string {
        return m_thread_name;
    }
    [[nodiscard]] auto GetThreadId() const -> uint32_t { return m_thread_id; }
    [[nodiscard]] auto GetFiberId() const -> uint32_t { return m_fiber_id; }
    [[nodiscard]] auto GetTime() const -> time_t { return m_time; }
    [[nodiscard]] auto GetContent() -> std::string { return m_content; }

    void SetLevel(LogLevel::Level level) { m_level = level; }

  private:
    LogLevel::Level m_level;    // 日志等级
    std::string m_file_name;    // 文件名
    uint32_t m_line;            // 行号
    uint32_t m_elapse{0};       // 程序启动开始到现在的毫秒数
    std::string m_thread_name;  // 线程名称
    uint32_t m_thread_id;       // 线程id
    uint32_t m_fiber_id;        // 携程id
    time_t m_time;              // 报告时间
    std::string m_content{};    // 日志内容
};
}  // namespace wtsclwq
