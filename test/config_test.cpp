/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-08 11:58:32
 * @LastEditTime: 2023-03-21 16:29:03
 */

#include "../src/include/config/config.h"

#include <cstddef>
#include <iostream>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "../src/include/log/log_manager.h"
#include "../src/include/util/cast_util.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"

const int port = 8080;

static auto logger = ROOT_LOGGER;
void print_yml(const YAML::Node &node, int level) {
    if (node.IsScalar()) {
        LOG_INFO(ROOT_LOGGER, node.Scalar() + " - " + node.Tag() + " - " +
                                  std::to_string(level));
    } else if (node.IsNull()) {
        LOG_INFO(ROOT_LOGGER,
                 "NULL - " + node.Tag() + " - " + std::to_string(level));
    } else if (node.IsMap()) {
        for (auto iter = node.begin(); iter != node.end(); iter++) {
            std::stringstream sss;
            sss << iter->first << " - " << iter->second.Tag() << " - " << level;
            LOG_INFO(ROOT_LOGGER, sss.str());
            print_yml(iter->second, level + 1);
        }
    } else if (node.IsSequence()) {
        for (size_t i = 0; i < node.size(); i++) {
            std::stringstream sss;
            sss << i << " - " << node.Tag() << " - " << level;
            LOG_INFO(ROOT_LOGGER, sss.str());
            print_yml(node[i], level + 1);
        }
    }
}

void test_yml() {
    YAML::Node root = YAML::LoadFile("/home/wtsclwq/desktop/config.yml");
    std::stringstream sss;
    sss << root;
    LOG_INFO(ROOT_LOGGER, sss.str());
    print_yml(root, 0);
}

void test_std_type() {
    auto g_int_val_config =
        wtsclwq::Config::Lookup<int>("system.port", 123, "system port");
    auto g_float_val_config =
        wtsclwq::Config::Lookup<float>("system.port", 99.9, "system port");
    auto g_double_val_config =
        wtsclwq::Config::Lookup("system.ratio", 16.79, "system ratio");
    auto g_int_vector_val_config = wtsclwq::Config::Lookup(
        "system.int_vector", std::vector<int>{1, 2, 3}, "a vector of int");
    auto g_int_list_val_config = wtsclwq::Config::Lookup(
        "system.int_list", std::list<int>{1, 2, 3}, "a list of int");
    auto g_int_set_val_config = wtsclwq::Config::Lookup(
        "system.int_set", std::set<int>{1, 1, 1, 2, 2, 2, 3}, "a set of int");
    auto g_int_unordered_set_val_config = wtsclwq::Config::Lookup(
        "system.int_unordered_set",
        std::unordered_set<int>{1, 1, 1, 2, 2, 2, 3}, "a unordered set of int");
    auto g_sim_map_str_int_config = wtsclwq::Config::Lookup(
        "system.ages_sim_map",
        std::map<std::string, int>{{"lwq", 888}, {"sxh", 999}, {"llb", 1010}},
        "a map of string to int");
    auto g_complex_map_str_int_config = wtsclwq::Config::Lookup(
        "system.grades_complex_map",
        std::map<std::string, std::vector<int>>{
            {"lwq", {1, 2, 3}}, {"sxh", {4, 5, 6}}, {"llb", {7, 8, 9}}},
        "a map of string to int");
    auto g_sim_unordered_map_str_int_config =
        wtsclwq::Config::Lookup("system.ages_sim_unordered_map",
                                std::unordered_map<std::string, int>{
                                    {"LWQ", 1}, {"SXH", 2}, {"LLB", 3}},
                                "a unordered map of string to int");
    std::ostringstream oss;
    LOG_INFO(ROOT_LOGGER, "before");

    oss << g_int_val_config->GetValue();
    LOG_INFO(ROOT_LOGGER, oss.str());
    oss.str("");
    oss << g_double_val_config->GetValue();
    LOG_INFO(ROOT_LOGGER, oss.str());
    oss.str("");
    for (auto &i : g_int_vector_val_config->GetValue()) {
        oss << "int vector " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_int_list_val_config->GetValue()) {
        oss << "int list " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_int_set_val_config->GetValue()) {
        oss << "int set " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_int_unordered_set_val_config->GetValue()) {
        oss << "int unordered set " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_sim_map_str_int_config->GetValue()) {
        oss << "string int sim map " << i.first << "," << i.second;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_complex_map_str_int_config->GetValue()) {
        oss << "string vec<int> complex map " << i.first;
        for (auto &j : i.second) {
            oss << " " << j;
        }
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_sim_unordered_map_str_int_config->GetValue()) {
        oss << "string int sim unordered map " << i.first << "," << i.second;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    YAML::Node root = YAML::LoadFile("/home/wtsclwq/desktop/config.yml");
    wtsclwq::Config::LoadFromYaml(root);

    LOG_INFO(ROOT_LOGGER, "after");

    oss << g_int_val_config->GetValue();
    LOG_INFO(ROOT_LOGGER, oss.str());
    oss.str("");
    oss << g_double_val_config->ToString();
    LOG_INFO(ROOT_LOGGER, oss.str());
    oss.str("");
    for (auto &i : g_int_vector_val_config->GetValue()) {
        oss << "int vector " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_int_list_val_config->GetValue()) {
        oss << "int list " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_int_set_val_config->GetValue()) {
        oss << "int set " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_int_unordered_set_val_config->GetValue()) {
        oss << "int unordered set " << i;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_sim_map_str_int_config->GetValue()) {
        oss << "string int sim map " << i.first << "," << i.second;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_complex_map_str_int_config->GetValue()) {
        oss << "string vec<int> complex map " << i.first;
        for (auto &j : i.second) {
            oss << " " << j;
        }
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
    for (auto &i : g_sim_unordered_map_str_int_config->GetValue()) {
        oss << "string int sim unordered map " << i.first << "," << i.second;
        LOG_INFO(ROOT_LOGGER, oss.str());
        oss.str("");
    }
}

void test_class() {
    auto g_person = wtsclwq::Config::Lookup("class.person", wtsclwq::Person(),
                                            "costum person class");
    g_person->AddListener(
        [](const wtsclwq::Person &old_val, const wtsclwq::Person &new_val) {
            LOG_INFO(ROOT_LOGGER,
                     "Callback test : old value = " + old_val.ToString() +
                         "new value = " + new_val.ToString());
        });
    LOG_INFO(ROOT_LOGGER, "before");
    LOG_INFO(ROOT_LOGGER,
             g_person->GetValue().ToString() + " - " + g_person->ToString());
    YAML::Node root = YAML::LoadFile("/home/wtsclwq/desktop/test.yml");
    wtsclwq::Config::LoadFromYaml(root);
    LOG_INFO(ROOT_LOGGER, "after");
    LOG_INFO(ROOT_LOGGER,
             g_person->GetValue().ToString() + " - " + g_person->ToString());
}

void test_yaml() {
    YAML::Node node;
    node["name"] = "lwq";
    node["age"] = 18;
    node["name"] = "sxh";
    std::cout << node << std::endl;
    // if (node.IsSequence()) {
    //     for (auto i : node) {
    //         std::cout << i.first << "  " << i.second << std::endl;
    //     }
    // }
    std::cout << node["name"] << std::endl;
}

void test_log_config() {
    auto g_logs_set =
        wtsclwq::Config::LookupByName<std::vector<wtsclwq::LoggerConfig>>(
            "logs");
    LOG_INFO(logger, "before");
    LOG_INFO(logger, std::to_string(g_logs_set->GetValue().size()));
    for (const auto &item : g_logs_set->GetValue()) {
        LOG_INFO(logger, item.name);
    }
    YAML::Node node = YAML::LoadFile("/home/wtsclwq/desktop/log_config.yml");
    wtsclwq::Config::LoadFromYaml(node);
    logger = ROOT_LOGGER;

    LOG_INFO(logger, "全局logger输出");
}

auto main() -> int {
    test_log_config();
    return 0;
}
