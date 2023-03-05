
#include "../include/log/log_level.h"

namespace wtsclwq {

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

}  // namespace wtsclwq
