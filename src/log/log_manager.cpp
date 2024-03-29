/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-21 17:47:53
 */

#include "../include/log/log_manager.h"

#include <memory>
#include <string>
#include <utility>

namespace wtsclwq {

// static对象在main()之前初始化
static LogIniter __log_init__;  // NOLINT

LogManager::LogManager() { Init({}); }

auto LogManager::GetGlobalLogger() -> Logger::ptr {
    return GetLogger("global");
}

auto LogManager::GetLogger(const std::string& name) -> Logger::ptr {
    ScopedLock<MutexType> lock(m_mutex);
    auto iter = m_logger_map.find(name);
    if (iter == m_logger_map.end()) {
        // 日志器不存在就返回全局默认日志器
        return m_logger_map.find("global")->second;
    }
    return iter->second;
}

void LogManager::Init(const std::vector<LoggerConfig>& logger_config_vec) {
    ScopedLock<MutexType> lock(m_mutex);

    m_logger_map.clear();

    for (const auto& logger_config : logger_config_vec) {
        // 删除已经存在的同名logger(更新)
        // !TODO(wtsclwq)
        /* 如果存在更新的情况，需要修改原logger的信息，因为shared_ptr还在其他地方被持有，不会自动析构logge
                 auto iter = m_logger_map.find(logger_config.name);
                if (iter != m_logger_map.end()) {
                    Logger::ptr logger = iter->second;
                }
         */
        auto logger = std::make_shared<Logger>(
            logger_config.name, logger_config.level, logger_config.pattern);
        for (const auto& appender_config : logger_config.appenders) {
            LogAppender::ptr appender;
            switch (appender_config.type) {
                case LogAppenderConfig::STDOUT:
                    appender = std::make_shared<StdoutLogAppender>(
                        appender_config.level);
                    break;
                case LogAppenderConfig::FILE:
                    appender = std::make_shared<FileLogAppender>(
                        appender_config.file, appender_config.level);
                    break;
                default:
                    std::cerr
                        << "LoggerManager::init exception 无效的 appender "
                           "配置值，appender.type="
                        << std::to_string(appender_config.type) << std::endl;
            }
            // 如果定义了appender的日志格式，为其创建专属的formatter
            // 否则在AddAppender时会自动设置为logger的默认formatter
            if (!appender_config.pattern.empty()) {
                appender->SetFormatter(
                    std::make_shared<LogFormatter>(appender_config.pattern));
            }
            logger->AddAppender(std::move(appender));
        }
        std::cout << "Logger创建成功：" + logger->GetName() << std::endl;

        m_logger_map.insert(
            std::make_pair(logger_config.name, std::move(logger)));
    }
    EnsureGlobalLogger();
}

void LogManager::EnsureGlobalLogger() {
    auto iter = m_logger_map.find("global");
    if (iter == m_logger_map.end()) {
        auto global_logger = std::make_shared<Logger>();
        global_logger->AddAppender(std::make_shared<StdoutLogAppender>());
        m_logger_map["global"] = std::move(global_logger);
    } else if (!iter->second) {
        iter->second = std::make_shared<Logger>();
        iter->second->AddAppender(std ::make_shared<StdoutLogAppender>());
    }
}

}  // namespace wtsclwq