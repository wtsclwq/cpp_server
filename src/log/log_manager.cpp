#include "../include/log/log_manager.h"

#include <memory>
#include <utility>

namespace wtsclwq {
LogManager::LogManager() { Init(); }

auto LogManager::GetGlobalLogger() -> Logger::ptr {
    return m_logger_map.find("global")->second;
}
void LogManager::Init() { EnsureGlobalLogger(); }

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