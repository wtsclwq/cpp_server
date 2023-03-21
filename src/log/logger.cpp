/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-20 23:46:11
 */
#include "../include/log/logger.h"

#include "../include/concurrency/lock.h"

namespace wtsclwq {

Logger::Logger()
    : m_name("default"), m_level(LogLevel::Level::DEBUG),
      m_pattern("[%d] [%p] [%N %t:%F] [%f:%l]%T%m%n"),
      m_formatter(std::make_shared<LogFormatter>(m_pattern)) {
    ;
}

Logger::Logger(std::string name, LogLevel::Level level, std::string pattern)
    : m_name(std::move(name)), m_level(level), m_pattern(std::move(pattern)),
      m_formatter(std::make_shared<LogFormatter>(m_pattern)) {}

auto Logger::GetName() const -> std::string { return m_name; }

auto Logger::GetLevel() const -> LogLevel::Level { return m_level; }

void Logger::AddAppender(LogAppender::ptr&& log_appender) {
    ScopedLock<MutexType> lock(m_mutex);
    if (!log_appender->GetFormatter()) {
        log_appender->SetFormatter(m_formatter);
    }
    m_log_appenders.emplace_back(std::move(log_appender));
}

void Logger::DelAppender(const LogAppender::ptr& log_appender) {
    ScopedLock<MutexType> lock(m_mutex);
    for (auto iter = m_log_appenders.begin(); iter != m_log_appenders.end();
         iter++) {
        if (*iter == log_appender) {
            m_log_appenders.erase(iter);
        }
    }
}

void Logger::Log(const LogEvent::ptr& log_event) {
    ScopedLock<MutexType> lock(m_mutex);
    if (log_event->GetLevel() >= m_level) {
        for (const auto& appender : m_log_appenders) {
            appender->Log(log_event);
        }
    }
}
}  // namespace wtsclwq
