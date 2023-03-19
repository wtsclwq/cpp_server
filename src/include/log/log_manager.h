/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-19 16:29:57
 */
#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "../config/config.h"
#include "../util/singleton.h"
#include "../util/thread_util.h"
#include "logger.h"

#define MAKE_LOG_EVENT(level, content)                         \
    std::make_shared<wtsclwq::LogEvent>(                       \
        __FILE__, __LINE__, content, wtsclwq::GetThreadName(), \
        wtsclwq::GetThreadId(), wtsclwq::GetFiberId(), time(nullptr), level)

#define LOG_BY_LEVEL(logger, level, message) \
    logger->Log(MAKE_LOG_EVENT(level, message))

#define LOG_DEBUG(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::DEBUG, message)

#define LOG_INFO(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::INFO, message)

#define LOG_WARN(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::WARN, message)

#define LOG_ERROR(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::ERROR, message)

#define LOG_FATAL(logger, message) \
    LOG_BY_LEVEL(logger, wtsclwq::LogLevel::Level::FATAL, message)

#define LOG_CUSTOM_LEVEL(logger, level, pattern, ...)                 \
    {                                                                 \
        char *buffer = new char;                                      \
        int length = asprintf(&buffer, pattern, ##__VA_ARGS__);       \
        if (length != -1) {                                           \
            LOG_BY_LEVEL(logger, level, std::string(buffer, length)); \
            delete buffer;                                            \
        }                                                             \
    }

#define LOG_CUSTOM_DEBUG(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::DEBUG, pattern, argv)

#define LOG_CUSTOM_INFO(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::INFO, pattern, argv)

#define LOG_CUSTOM_WARN(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::WARN, pattern, argv)

#define LOG_CUSTOM_ERROR(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::ERROR, pattern, argv)

#define LOG_CUSTOM_FATAL(logger, pattern, argv...) \
    LOG_CUSTOM_LEVEL(logger, wtsclwq::LogLevel::Level::FATAL, pattern, argv)

// 获取全局root logger
#define ROOT_LOGGER \
    wtsclwq::SingltonLogManager::GetInstancePtr()->GetGlobalLogger()
// 根据 name 查找 logger
#define GET_LOGGER_BY_NAME(name) \
    wtsclwq::SingltonLogManager::GetInstancePtr()->GetLogger(name)

namespace wtsclwq {

class LogManager {
  public:
    using ptr = std::shared_ptr<LogManager>;

    friend struct LogIniter;
    explicit LogManager();
    auto GetLogger(const std::string &name) -> Logger::ptr;
    auto GetGlobalLogger() -> Logger::ptr;

  private:
    void Init();
    void EnsureGlobalLogger();

    std::map<std::string, Logger::ptr> m_logger_map;
};

using SingltonLogManager = SingletonPtr<LogManager>;

struct LogIniter {
    LogIniter() {
        auto log_congig_set = Config::Lookup<std::vector<LoggerConfig>>(
            "logs", {}, "日志器配置项目集合");
        log_congig_set->AddListener([](const std::vector<LoggerConfig> &,
                                       const std::vector<LoggerConfig> &) {
            LOG_INFO(ROOT_LOGGER, "日志器配置变动，重新初始化日志器");
            SingltonLogManager::GetInstancePtr()->Init();
        });
    }
};

// static对象在main()之前初始化
static LogIniter __log_init__;  // NOLINT

}  // namespace wtsclwq
