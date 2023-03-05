#pragma once

#include <map>
#include <memory>

#include "../util/singleton.h"
#include "logger.h"

namespace wtsclwq {

class LogManager {
   public:
    using ptr = std::shared_ptr<LogManager>;

    explicit LogManager();
    auto GetLogger(const std::string &name) -> Logger::ptr;
    auto GetGlobalLogger() -> Logger::ptr;

   private:
    void Init();
    void EnsureGlobalLogger();

    std::map<std::string, Logger::ptr> m_logger_map;
};

using SingltonLogManager = SingletonPtr<LogManager>;
}  // namespace wtsclwq
