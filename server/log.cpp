//
// Created by wtsclwq on 23-3-2.
//

#include "include/log.h"

#include <memory>
namespace wtsclwq {

/**
 * %p 输出日志等级
 * %f 输出文件名
 * %l 输出行号
 * %d 输出日志时间
 * %t 输出线程号
 * %F 输出协程号
 * %m 输出日志消息
 * %n 输出换行
 * %% 输出百分号
 * %T 输出制表符
 * */
thread_local static std::map<char, LogFormatter::FormatItem::ptr>
    format_item_map{
#define FN(CH, ITEM_NAME) {CH, std::make_shared<ITEM_NAME>()}
        FN('p', LevelFormatItem),       FN('f', FilenameFormatItem),
        FN('l', LineFormatItem),        FN('d', TimeFormatItem),
        FN('t', ThreadIDFormatItem),    FN('F', FiberIDFormatItem),
        FN('m', ContentFormatItem),     FN('n', NewLineFormatItem),
        FN('%', PercentSignFormatItem), FN('T', TabFormatItem),
#undef FN
    };

auto LogLevel::ToString(LogLevel::Level level) -> std::string {
    std::string result;
    switch (level) {
        case DEBUG:
            result = "DEBUG";
            break;
        case INFO:
            result = "INFO";
            break;
        case WARN:
            result = "WARN";
            break;
        case ERROR:
            result = "ERROR";
            break;
        case FATAL:
            result = "FATAL";
            break;
        case UNKNOWN:
            result = "UNKNOWN";
            break;
    }
    return result;
}

Logger::Logger()
    : m_name("default"), m_level(LogLevel::Level::DEBUG),
      m_pattern("[%d] [%p] [T:%t F:%F]%T%m%n") {
    m_formatter.reset(new LogFormatter(m_pattern));
}

Logger::Logger(const std::string name, LogLevel::Level level,
               const std::string& pattern)
    : m_name(name), m_level(level), m_pattern(pattern) {
    m_formatter.reset(new LogFormatter(pattern));
}

void Logger::Log(LogLevel::Level level, const LogEvent::ptr log_event) {
    if (level >= m_level) {
        for (auto& i : m_log_appenders) {
            i->Log(level, log_event);
        }
    }
}

void Logger::Debug(const LogEvent::ptr log_event) {
    Log(LogLevel::Level::DEBUG, log_event);
}

void Logger::InFo(const LogEvent::ptr log_event) {
    Log(LogLevel::Level::INFO, log_event);
}

void Logger::Warn(const LogEvent::ptr log_event) {
    Log(LogLevel::Level::WARN, log_event);
}

void Logger::Error(const LogEvent::ptr log_event) {
    Log(LogLevel::Level::ERROR, log_event);
}

void Logger::Fatal(const LogEvent::ptr log_event) {
    Log(LogLevel::Level::FATAL, log_event);
}

void Logger::AddAppender(LogAppender::ptr log_appender) {
    if (!log_appender->GetFormatter()) {
        log_appender->SetFormatter(m_formatter);
    }
    m_log_appenders.push_back(log_appender);
}

void Logger::DelAppender(LogAppender::ptr log_appender) {
    auto iter =
        std::find(m_log_appenders.begin(), m_log_appenders.end(), log_appender);
    if (iter != m_log_appenders.end()) {
        //        (*iter).reset();
        m_log_appenders.erase(iter);
    }
}

FileLogAppender::FileLogAppender(const std::string& file_name,
                                 LogLevel::Level level = LogLevel::Level::DEBUG)
    : LogAppender(level), m_file_name(file_name) {
    ReOpen();
}

void FileLogAppender::Log(LogLevel::Level level, LogEvent::ptr log_event) {
    if (level >= m_level) {
        ReOpen();
        m_file_stream << m_formatter->Format(log_event);
        m_file_stream.flush();
    }
}

auto FileLogAppender::ReOpen() -> bool {
    if (m_file_stream) {
        m_file_stream.close();
    }
    m_file_stream.open(m_file_name);
    return !!m_file_stream;  // 有意思，两次逻辑取反获得bool值
}

void StdoutLogAppender::Log(LogLevel::Level level, LogEvent::ptr log_event) {
    if (level >= m_level) {
        std::cout << m_formatter->Format(log_event);
        std::cout.flush();
    }
}

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
    Init();
}

auto LogFormatter::Format(LogEvent::ptr log_event) -> std::string {
    std::ostringstream oss;
    for (auto& i : m_items) {
        i->Format(oss, log_event);
    }
    return oss.str();
}

void LogFormatter::Init() {
    enum PARSE_STATUS {
        SCAN_STATUS,    // 扫描普通字符
        CREATE_STATUS,  // 扫描到 %，处理占位符
    };
    PARSE_STATUS STATUS = SCAN_STATUS;
    size_t str_begin = 0, str_end = 0;
    //    std::vector<char> item_list;
    for (size_t i = 0; i < m_pattern.length(); i++) {
        switch (STATUS) {
            // 普通扫描分支，将扫描到普通字符串创建对应的普通字符处理对象后填入m_format_item_list中
            case SCAN_STATUS:
                // 扫描记录普通字符的开始结束位置
                str_begin = i;
                for (str_end = i; str_end < m_pattern.length(); str_end++) {
                    // 扫描到 % 结束普通字符串查找，将 STATUS
                    // 赋值为占位符处理状态，等待后续处理后进入占位符处理状态
                    if (m_pattern[str_end] == '%') {
                        STATUS = CREATE_STATUS;
                        break;
                    }
                }
                i = str_end;
                m_items.push_back(std::make_shared<PlainFormatItem>(
                    m_pattern.substr(str_begin, str_end - str_begin)));
                break;
            // 处理占位符
            case CREATE_STATUS:
                assert(!format_item_map.empty(),
                       "format_item_map 没有被正确的初始化");
                auto iter = format_item_map.find(m_pattern[i]);
                if (iter == format_item_map.end()) {
                    m_items.push_back(
                        std::make_shared<PlainFormatItem>("<error format>"));
                } else {
                    m_items.push_back(iter->second);
                }
                STATUS = SCAN_STATUS;
                break;
        }
    }
}

}  // namespace wtsclwq
