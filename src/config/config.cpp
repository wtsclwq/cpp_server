/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-07 23:33:58
 * @LastEditTime: 2023-03-12 00:43:30
 */
#include "../include/config/config.h"

#include <algorithm>
#include <cctype>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "../include/log/log_manager.h"
#include "yaml-cpp/node/node.h"
namespace wtsclwq {

auto Config::LookupBase(const std::string &name) -> ConfigVarBase::ptr {
    auto iter = GetData().find(name);
    return iter == GetData().end() ? nullptr : iter->second;
}

void Config::LoadFromYaml(const YAML::Node &root) {
    std::list<std::pair<std::string, YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);

    for (const auto &item : all_nodes) {
        auto key = item.first;
        // 根结点
        if (key.empty()) {
            continue;
        }
        // 为什么要用base？因为不能根据name就判断出配置项的类型
        auto var_base = LookupBase(key);
        if (var_base) {
            if (item.second.IsScalar()) {
                var_base->FromString(item.second.Scalar());

            } else {
                std::stringstream sss;
                sss << item.second;
                var_base->FromString(sss.str());
            }
        }
    }
}

void Config::ListAllMember(const std::string &name, const YAML::Node &node,
                           std::list<std::pair<std::string, YAML::Node>> &output) {
    if (name.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789") !=
        std::string::npos) {
        std::ostringstream oss;
        oss << "配置项名称非法" << name << ":" << node;
        LOG_ERROR(ROOT_LOGGER, oss.str());
        return;
    }
    // 将 YAML::Node 存入 output
    auto output_iter =
        std::find_if(output.begin(), output.end(),
                     [&name](const std::pair<std::string, YAML::Node> &item) {
                         return item.first == name;
                     });
    if (output_iter != output.end()) {
        output_iter->second = node;
    } else {
        output.emplace_back(name, node);
    }
    // 当 YAML::Node 为映射型节点，使用迭代器遍历
    if (node.IsMap()) {
        for (auto iter = node.begin(); iter != node.end(); ++iter) {
            ListAllMember(
                name.empty() ? iter->first.Scalar() : name + "." + iter->first.Scalar(),
                iter->second, output);
        }
    } /* else if (node.IsSequence()) {  // 当 YAML::Node 为序列型节点，使用下标遍历
        for (size_t i = 0; i < node.size(); ++i) {
            ListAllMember(name + "." + std::to_string(i), node[i], output);
        }
    } */
}
}  // namespace wtsclwq