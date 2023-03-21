/*
 * @Description:
 * @LastEditTime: 2023-03-21 12:56:00
 */
#pragma once
#include <vector>

#include "../log/log_level.h"
#include "../util/cast_util.h"

namespace wtsclwq {

struct LogAppenderConfig {
    enum Type { STDOUT = 0, FILE = 1 };
    LogAppenderConfig::Type type{Type::STDOUT};       // 目的地的类型
    LogLevel::Level level{LogLevel::Level::UNKNOWN};  // 日志等级
    std::string pattern;  // 目的地的日志格式
    std::string file;     // 目的地的目标文件path,仅在FILE时有效

    auto operator==(const LogAppenderConfig& other) const -> bool {
        return type == other.type && level == other.level &&
               pattern == other.pattern && file == other.file;
    }
};

struct LoggerConfig {
    std::string name;                                 // 日志器名称
    LogLevel::Level level{LogLevel::Level::UNKNOWN};  // 日志有效等级
    std::string pattern;                              // 日志格式
    std::vector<LogAppenderConfig> appenders;         // 目的地集合

    auto operator==(const LoggerConfig& other) const -> bool {
        return name == other.name;
    }
};

inline auto parse_attribute(const YAML::Node& node,
                            const std::string& attribute) -> std::string {
    return node[attribute] ? node[attribute].as<std::string>() : "";
}

/*
 * 针对std::set<LoggerConfig>的偏特化
 */
template <>
class LexicalCast<std::string, std::vector<LoggerConfig>> {
  public:
    auto operator()(const std::string& logger_configs_str)  // NOLINT
        -> std::vector<LoggerConfig> {
        YAML::Node node = YAML::Load(logger_configs_str);

        std::vector<LoggerConfig> logger_configs_set;
        if (node.IsSequence()) {
            for (const auto logger_config_node : node) {
                LoggerConfig logger_config;
                logger_config.name =
                    parse_attribute(logger_config_node, "name");

                logger_config.level = LogLevel::FromString(
                    parse_attribute(logger_config_node, "level"));

                logger_config.pattern =
                    parse_attribute(logger_config_node, "pattern");

                if (logger_config_node["appenders"] &&
                    logger_config_node["appenders"].IsSequence()) {
                    for (const auto& appender_config_node :
                         logger_config_node["appenders"]) {
                        LogAppenderConfig log_appender_config;

                        auto type_str =
                            parse_attribute(appender_config_node, "type");
                        if (type_str == "STDOUT") {
                            log_appender_config.type =
                                LogAppenderConfig::Type::STDOUT;
                        } else if (type_str == "FILE") {
                            log_appender_config.type =
                                LogAppenderConfig::Type::FILE;
                        } else {
                            log_appender_config.type =
                                LogAppenderConfig::Type::STDOUT;
                            std::cerr << "目的地类型非法，默认设为STDOUT"
                                      << std::endl;
                        }
                        log_appender_config.file =
                            parse_attribute(appender_config_node, "file");

                        log_appender_config.level = LogLevel::FromString(
                            parse_attribute(appender_config_node, "level"));

                        log_appender_config.pattern =
                            parse_attribute(appender_config_node, "pattern");

                        logger_config.appenders.push_back(log_appender_config);
                    }
                }
                logger_configs_set.push_back(logger_config);
            }
        }
        return logger_configs_set;
    }
};

/*
 * 针对std::set<LoggerConfig>的偏特化
 */
template <>
class LexicalCast<std::vector<LoggerConfig>, std::string> {
  public:
    auto operator()(const std::vector<LoggerConfig>& logger_configs_set)
        -> std::string {
        YAML::Node logger_configs_node;

        for (const auto& loggger_config : logger_configs_set) {
            YAML::Node node;
            node["name"] = loggger_config.name;
            node["level"] = static_cast<int>(loggger_config.level);
            node["pattern"] = loggger_config.pattern;
            YAML::Node appenders_seq_node;
            for (const auto& appender_config : loggger_config.appenders) {
                YAML::Node appender_node;
                appender_node["type"] = static_cast<int>(appender_config.type);
                appender_node["file"] = appender_config.file;
                appender_node["level"] =
                    static_cast<int>(appender_config.level);
                appender_node["pattern"] = appender_config.pattern;
                appenders_seq_node.push_back(appender_node);
            }
            node["appenders"] = appenders_seq_node;
            logger_configs_node.push_back(node);
        }
        std::stringstream sstream;
        sstream << logger_configs_node;
        return sstream.str();
    }
};
}  // namespace wtsclwq