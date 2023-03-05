#pragma once

#include <string>

namespace wtsclwq {

/**
 * 日志级别
 */
class LogLevel {
 public:
  enum Level {
    UNKNOWN = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERROR = 4,
    FATAL = 5
  };
  static auto ToString(LogLevel::Level level) -> std::string;
};

}  // namespace wtsclwq
