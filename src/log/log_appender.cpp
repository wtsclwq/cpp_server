#include "../include/log/log_appender.h"

namespace wtsclwq {
LogAppender::LogAppender(LogLevel::Level level) : m_level(level) {}

FileLogAppender::FileLogAppender(std::string file_name, LogLevel::Level level)
    : LogAppender(level), m_file_name(std::move(file_name)) {
    ReOpen();
}

void FileLogAppender::Log(const LogEvent::ptr &log_event) {
    if (log_event->GetLevel() >= this->GetLevel()) {
        m_file_stream << this->GetFormatter()->Format(log_event);
        m_file_stream.flush();
    }}

auto FileLogAppender::ReOpen() -> bool {
    if (!m_file_stream) {
        m_file_stream.close();
    }
    // 设置stream的打开模式为[输出+追加]
    m_file_stream.open(m_file_name, std::ios::app | std::ios::out);
    return !!m_file_stream;  // 有意思，两次逻辑取反获得bool值
}

StdoutLogAppender::StdoutLogAppender(LogLevel::Level level) : LogAppender(level) {}

void StdoutLogAppender::Log(const LogEvent::ptr &log_event) {
    if (log_event->GetLevel() >= this->GetLevel()) {
        std::cout << this->GetFormatter()->Format(log_event);
        std::cout.flush();
    }
}
}  // namespace wtsclwq
