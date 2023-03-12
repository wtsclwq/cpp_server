/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-12 15:22:09
 */
#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "../config/config.h"
#include "../util/singleton.h"
#include "logger.h"

// 获取全局root logger
#define ROOT_LOGGER wtsclwq::SingltonLogManager::GetInstancePtr()->GetGlobalLogger()
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
        auto log_congig_set =
            Config::Lookup<std::vector<LoggerConfig>>("logs", {}, "日志器配置项目集合");
        log_congig_set->AddListener(
            [](const std::vector<LoggerConfig> &, const std::vector<LoggerConfig> &) {
                LOG_INFO(ROOT_LOGGER, "日志器配置变动，重新初始化日志器");
                SingltonLogManager::GetInstancePtr()->Init();
            });
    }
};

// static对象在main()之前初始化
static LogIniter __log_init__;  // NOLINT

}  // namespace wtsclwq
