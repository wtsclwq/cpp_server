/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-12 12:09:20
 */
#include "../include/log/logger.h"

namespace wtsclwq {

Logger::Logger()
    : m_name("default"), m_level(LogLevel::Level::DEBUG),
      m_pattern("[%d] [%p] [%t:%F] [%f:%l]%T%m%n"),
      m_formatter(std::make_shared<LogFormatter>(m_pattern)) {
    ;
}

Logger::Logger(std::string name, LogLevel::Level level, std::string pattern)
    : m_name(std::move(name)), m_level(level), m_pattern(std::move(pattern)),
      m_formatter(std::make_shared<LogFormatter>(m_pattern)) {}

void Logger::Log(const LogEvent::ptr& log_event) {
    if (log_event->GetLevel() >= m_level) {
        for (const auto& appender : m_log_appenders) {
            appender->Log(log_event);
        }
    }
}

void Logger::AddAppender(LogAppender::ptr&& log_appender) {
    if (!log_appender->GetFormatter()) {
        log_appender->SetFormatter(m_formatter);
    }
    m_log_appenders.emplace_back(std::move(log_appender));
}

void Logger::DelAppender(const LogAppender::ptr& log_appender) {
    for (auto iter = m_log_appenders.begin(); iter != m_log_appenders.end(); iter++) {
        if (*iter == log_appender) {
            m_log_appenders.erase(iter);
        }
    }
}
}  // namespace wtsclwq
