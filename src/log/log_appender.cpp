/*
 * @Description:
 * @LastEditTime: 2023-03-21 16:31:14
 */
#include "../include/log/log_appender.h"

#include <mutex>

#include "../include/concurrency/lock.h"

namespace wtsclwq {
LogAppender::LogAppender(LogLevel::Level level) : m_level(level) {}

auto LogAppender::GetFormatter() const -> LogFormatter::ptr {
    return m_formatter;
}

void LogAppender::SetFormatter(LogFormatter::ptr formatter) {
    ScopedLock<MutexType> lock(m_mutex);
    m_formatter = std::move(formatter);
}

auto LogAppender::GetLevel() const -> LogLevel::Level { return m_level; }

auto LogAppender::SetLevel(LogLevel::Level level) { m_level = level; }

auto LogAppender::GetMutex() -> MutexType & { return m_mutex; }

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

FileLogAppender::FileLogAppender(std::string file_name, LogLevel::Level level)
    : LogAppender(level), m_file_name(std::move(file_name)) {
    ReOpen();
}

auto FileLogAppender::ReOpen() -> bool {
    ScopedLock<MutexType> lock(GetMutex());
    if (!m_file_stream) {
        m_file_stream.close();
    }
    // 设置stream的打开模式为[输出+追加]
    m_file_stream.open(m_file_name, std::ios::app | std::ios::out);
    return !!m_file_stream;  // 有意思，两次逻辑取反获得bool值
}

void FileLogAppender::Log(const LogEvent::ptr &log_event) {
    ScopedLock<MutexType> lock(GetMutex());
    if (log_event->GetLevel() >= this->GetLevel()) {
        m_file_stream << this->GetFormatter()->Format(log_event);
        m_file_stream.flush();
    }
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

StdoutLogAppender::StdoutLogAppender(LogLevel::Level level)
    : LogAppender(level) {}

void StdoutLogAppender::Log(const LogEvent::ptr &log_event) {
    ScopedLock<MutexType> lock(GetMutex());
    if (log_event->GetLevel() >= this->GetLevel()) {
        std::cout << this->GetFormatter()->Format(log_event);
        std::cout.flush();
    }
}
}  // namespace wtsclwq
