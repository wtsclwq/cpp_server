#include "../include/log/log_formatter.h"

#include <iostream>
#include <map>

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
static thread_local std::map<char, LogFormatter::FormatItem::ptr> format_item_map{//NOLINT
    {'p', std::make_shared<LevelFormatItem>()},
    {'f', std::make_shared<FilenameFormatItem>()},
    {'l', std::make_shared<LineFormatItem>()},
    {'d', std::make_shared<TimeFormatItem>()},
    {'t', std::make_shared<ThreadIDFormatItem>()},
    {'F', std::make_shared<FiberIDFormatItem>()},
    {'m', std::make_shared<ContentFormatItem>()},
    {'n', std::make_shared<NewLineFormatItem>()},
    {'%', std::make_shared<PercentSignFormatItem>()},
    {'T', std::make_shared<TabFormatItem>()},
};

LogFormatter::LogFormatter(std::string pattern) : m_pattern(std::move(pattern)) {
    Init();
}

auto LogFormatter::Format(const LogEvent::ptr& log_event) -> std::string {
    std::ostringstream oss;
    for (auto& item : m_items) {
        item->Format(oss, log_event);
    }
    return oss.str();
}

void LogFormatter::Init() {
    enum PARSE_STATUS {
        SCAN_STATUS,    // 扫描普通字符
        CREATE_STATUS,  // 扫描到 %，处理占位符
    };
    PARSE_STATUS STATUS = SCAN_STATUS;
    size_t str_begin = 0;
    size_t str_end = 0;
    for (size_t i = 0; i < m_pattern.length(); i++) {
        switch (STATUS) {
            // 普通扫描分支，将扫描到普通字符串创建对应的普通字符处理对象后填入m_format_item_list中
            case SCAN_STATUS:
                str_begin = i;  // 扫描记录普通字符的开始结束位置
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
                assert(("format_item_map没有被正确的初始化", !format_item_map.empty()));
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
