/*
 * @Author: mikey.zhaopeng
 * @Date: 2023-03-Fr 04:47:46
 * @Last Modified by:   mikey.zhaopeng
 * @Last Modified time: 2023-03-Fr 04:47:46
 */
#pragma once

#include <list>
#include <map>
#include <memory>
#include <utility>

#include "config_var.h"
#include "config_var_base.h"
namespace wtsclwq {

class Config {
   public:
    // map里存储基类，这样可以避免编译时就要实例化模板类
    using ConfigValMap = std::map<std::string, ConfigVarBase::ptr>;

    template <typename T>
    static auto Lookup(const std::string &name, const T &default_value,
                       const std::string &description) -> typename ConfigVar<T>::ptr;

    template <typename T>
    static auto LookupByName(const std::string &name) -> typename ConfigVar<T>::ptr;

    static void LoadFromYaml(const YAML::Node &root);
    static auto LookupBase(const std::string &name) -> ConfigVarBase::ptr;

   private:
    static void ListAllMember(const std::string &name, const YAML::Node &node,
                              std::list<std::pair<std::string, YAML::Node>> &output);
    static auto GetData() -> ConfigValMap & {
        static ConfigValMap s_data;
        return s_data;
    } 
};

template <typename T>
auto Config::Lookup(const std::string &name, const T &default_value,
                    const std::string &description) -> typename ConfigVar<T>::ptr {
    auto iter = GetData().find(name);
    if (iter != GetData().end()) {
        auto temp = std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
        // 转换成功，智能指针非空
        if (temp) {
            return temp;
        }
        std::cerr << "Lookup name = " + name + "exists but type is not" +
                         typeid(T).name() + "real type is " +
                         iter->second->GetTypeName() + " " + iter->second->ToString();
        return nullptr;
    }

    // 没找到的话，添加一个默认配置
    // 判断配置项名称是否合法
    if (name.find_first_not_of(
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ._0123456789") !=
        std::string::npos) {
        std::cerr << "配置项名称非法" + name;
        throw std::invalid_argument(name);
    }

    auto val = std::make_shared<ConfigVar<T>>(name, default_value, description);
    GetData()[name] = val;
    return val;
}

template <typename T>
auto Config::LookupByName(const std::string &name) -> typename ConfigVar<T>::ptr {
    auto iter = GetData().find(name);
    if (iter == GetData().end()) {
        return nullptr;
    }
    return std::dynamic_pointer_cast<ConfigVar<T>>(iter->second);
}
}  // namespace wtsclwq
