/*
 * @Description:
 * @Author: wtsclwq
 * @Date: 2023-03-10 18:29:16
 * @LastEditTime: 2023-03-11 16:33:54
 * @LastEditors: Please set LastEditors
 */

#pragma once
#include <yaml-cpp/yaml.h>

#include <cstddef>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../log/log_manager.h"
#include "boost/lexical_cast.hpp"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
namespace wtsclwq {

/*
 * From F to T
 * 基础类型之间相互转换
 */
template <typename F, typename T>
class LexicalCast {
   public:
    /**
     * @description: 重载函数调用运算符()，利用boost/lexical_cast转换基础类型
     * @author: wtsclwq
     */
    auto operator()(const F& from) -> T { return boost::lexical_cast<T>(from); }
};

/*
 * 针对std::vector<T>的偏特化，当ConfigVar<T>中的T是std::vector<T>时，
 * FromStr就会被替换为当前的模板类,成为From String To Vector
 */
template <typename T>
class LexicalCast<std::string, std::vector<T>> {
   public:
    auto operator()(const std::string& val_str) -> std::vector<T> {
        YAML::Node node;
        // 调用 YAML::Load 解析传入的字符串，解析失败会抛出异常
        node = YAML::Load(val_str);
        std::vector<T> config_val_vector;
        if (node.IsSequence()) {
            // 检查解析后的 node 是否是一个序列型 YAML::Node
            std::stringstream sstream;
            for (const auto& item : node) {
                sstream.str("");
                // 利用 YAML::Node 实现的 operator<<() 将 node 转换为字符串
                sstream << item;
                // 递归解析，直到 T 为基本类型,
                // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
                // vector、set或者map都可以自动匹配到专属的偏特化模板类
                config_val_vector.push_back(LexicalCast<std::string, T>()(sstream.str()));
            }
        } else {
            LOG_INFO(ROOT_LOGGER,
                     "LexicalCast<std::string, std::vector>::operator() exception "
                     "<val_str> is not a YAML sequence");
        }
        return config_val_vector;
    }
};

/*
 * 针对std::vector<T>的偏特化，当ConfigVar<T>中的T是std::vector<T>时，
 * ToStr就会被替换为当前的模板类,成为From Vector To String
 */
template <typename T>
class LexicalCast<std::vector<T>, std::string> {
   public:
    auto operator()(const std::vector<T>& config_var_vec) -> std::string {
        YAML::Node node;
        for (auto& item : config_var_vec) {
            // 递归解析，直到 T 为基本类型,
            // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
            // vector、set或者map都可以自动匹配到专属的偏特化模板类
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
        }
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

/*
 * 针对std::list<T>的偏特化，当ConfigVar<T>中的T是std::list<T>时，
 * FromStr就会被替换为当前的模板类,成为From String To List
 */
template <typename T>
class LexicalCast<std::string, std::list<T>> {
   public:
    auto operator()(const std::string& val_str) -> std::list<T> {
        YAML::Node node;
        // 调用 YAML::Load 解析传入的字符串，解析失败会抛出异常
        node = YAML::Load(val_str);
        std::list<T> config_val_list;
        if (node.IsSequence()) {
            // 检查解析后的 node 是否是一个序列型 YAML::Node
            std::stringstream sstream;
            for (const auto& item : node) {
                sstream.str("");
                // 利用 YAML::Node 实现的 operator<<() 将 node 转换为字符串
                sstream << item;
                // 递归解析，直到 T 为基本类型,
                // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
                // vector、set或者map都可以自动匹配到专属的偏特化模板类
                config_val_list.push_back(LexicalCast<std::string, T>()(sstream.str()));
            }
        } else {
            LOG_INFO(ROOT_LOGGER,
                     "LexicalCast<std::string, std::list>::operator() exception "
                     "<val_str> is not a YAML sequence");
        }
        return config_val_list;
    }
};

/*
 * 针对std::list<T>的偏特化，当ConfigVar<T>中的T是std::list<T>时，
 * ToStr就会被替换为当前的模板类,成为From List To String
 */
template <typename T>
class LexicalCast<std::list<T>, std::string> {
   public:
    auto operator()(const std::list<T>& config_val_list) -> std::string {
        YAML::Node node;
        for (auto& item : config_val_list) {
            // 递归解析，直到 T 为基本类型,
            // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
            // vector、set或者map都可以自动匹配到专属的偏特化模板类
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
        }
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

/*
 * 针对std::set<T>的偏特化，当ConfigVar<T>中的T是std::set<T>时，
 * FromStr就会被替换为当前的模板类,成为From String To set
 */
template <typename T>
class LexicalCast<std::string, std::set<T>> {
   public:
    auto operator()(const std::string& val_str) -> std::set<T> {
        YAML::Node node;
        // 调用 YAML::Load 解析传入的字符串，解析失败会抛出异常
        node = YAML::Load(val_str);
        std::set<T> config_val_set;
        if (node.IsSequence()) {
            // 检查解析后的 node 是否是一个序列型 YAML::Node
            std::stringstream sstream;
            for (const auto& item : node) {
                sstream.str("");
                // 利用 YAML::Node 实现的 operator<<() 将 node 转换为字符串
                sstream << item;
                // 递归解析，直到 T 为基本类型,
                // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
                // vector、set或者map都可以自动匹配到专属的偏特化模板类
                config_val_set.insert(LexicalCast<std::string, T>()(sstream.str()));
            }
        } else {
            LOG_INFO(ROOT_LOGGER,
                     "LexicalCast<std::string, std::set>::operator() exception "
                     "<val_str> is not a YAML sequence");
        }
        return config_val_set;
    }
};

/*
 * 针对std::set<T>的偏特化，当ConfigVar<T>中的T是std::set<T>时，
 * ToStr就会被替换为当前的模板类,成为From Set To String
 */
template <typename T>
class LexicalCast<std::set<T>, std::string> {
   public:
    auto operator()(const std::set<T>& config_val_set) -> std::string {
        YAML::Node node;
        for (auto& item : config_val_set) {
            // 递归解析，直到 T 为基本类型,
            // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
            // vector、set或者map都可以自动匹配到专属的偏特化模板类
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
        }
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

/*
 * 针对std::set<T>的偏特化，当ConfigVar<T>中的T是std::set<T>时，
 * FromStr就会被替换为当前的模板类,成为From String To set
 */
template <typename T>
class LexicalCast<std::string, std::unordered_set<T>> {
   public:
    auto operator()(const std::string& val_str) -> std::unordered_set<T> {
        YAML::Node node;
        // 调用 YAML::Load 解析传入的字符串，解析失败会抛出异常
        node = YAML::Load(val_str);
        std::unordered_set<T> config_val_unordered_set;
        if (node.IsSequence()) {
            // 检查解析后的 node 是否是一个序列型 YAML::Node
            std::stringstream sstream;
            for (const auto& item : node) {
                sstream.str("");
                // 利用 YAML::Node 实现的 operator<<() 将 node 转换为字符串
                sstream << item;
                // 递归解析，直到 T 为基本类型,
                // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
                // vector、set或者map都可以自动匹配到专属的偏特化模板类
                config_val_unordered_set.insert(
                    LexicalCast<std::string, T>()(sstream.str()));
            }
        } else {
            LOG_INFO(ROOT_LOGGER,
                     "LexicalCast<std::string, std::unordered_set>::operator() exception "
                     "<val_str> is not a YAML sequence");
        }
        return config_val_unordered_set;
    }
};

/*
 * 针对std::unordered_set<T>的偏特化，当ConfigVar<T>中的T是std::unordered_set<T>时，
 * ToStr就会被替换为当前的模板类,成为From Unordered_set To String
 */
template <typename T>
class LexicalCast<std::unordered_set<T>, std::string> {
   public:
    auto operator()(const std::unordered_set<T>& config_val_set) -> std::string {
        YAML::Node node;
        for (auto& item : config_val_set) {
            // 递归解析，直到 T 为基本类型,
            // 因为node可能会是嵌套的，可能嵌套了vector、map、set或者是基础类型，
            // vector、set或者map都可以自动匹配到专属的偏特化模板类
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(item)));
        }
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

/*
 * 针对std::map<T>的偏特化，当ConfigVar<T>中的T是std::map<T>时，
 * ToStr就会被替换为当前的模板类,成为From String To Map
 */
template <typename T>
class LexicalCast<std::string, std::map<std::string, T>> {
   public:
    auto operator()(const std::string& val_str) -> std::map<std::string, T> {
        YAML::Node node = YAML::Load(val_str);
        std::map<std::string, T> config_val_map;
        if (node.IsMap()) {
            std::stringstream sstream;
            for (auto iter : node) {
                sstream.str("");
                sstream << iter.second;
                config_val_map.insert(std::make_pair(
                    iter.first.Scalar(), LexicalCast<std::string, T>()(sstream.str())));
            }
        } else {
            LOG_INFO(ROOT_LOGGER,
                     "LexicalCast<std::string, std::map>::operator() exception "
                     "<val_str> is not a YAML map");
        }
        return config_val_map;
    }
};

/*
 * 针对std::map<T>的偏特化，当ConfigVar<T>中的T是std::map<T>时，
 * ToStr就会被替换为当前的模板类,成为From Map To String
 */
template <typename T>
class LexicalCast<std::map<std::string, T>, std::string> {
   public:
    auto operator()(const std::map<std::string, T>& config_val_map) -> std::string {
        YAML::Node node;
        for (auto& item : config_val_map) {
            node[item.first] = YAML::Load(LexicalCast<T, std::string>()(item.second));
        }
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

/*
 * 针对std::unordered_map<T>的偏特化，当ConfigVar<T>中的T是std::unordered_map<T>时，
 * ToStr就会被替换为当前的模板类,成为From String To unordered_map
 */
template <typename T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
   public:
    auto operator()(const std::string& val_str) -> std::unordered_map<std::string, T> {
        YAML::Node node = YAML::Load(val_str);
        std::unordered_map<std::string, T> config_val_unordered_map;
        if (node.IsMap()) {
            std::stringstream sstream;
            for (auto iter : node) {
                sstream.str("");
                sstream << iter.second;
                config_val_unordered_map.insert(std::make_pair(
                    iter.first.Scalar(), LexicalCast<std::string, T>()(sstream.str())));
            }
        } else {
            LOG_INFO(ROOT_LOGGER,
                     "LexicalCast<std::string, std::map>::operator() exception "
                     "<val_str> is not a YAML map");
        }
        return config_val_unordered_map;
    }
};

/*
 * 针对std::unordered_map<T>的偏特化，当ConfigVar<T>中的T是std::unordered_map<T>时，
 * ToStr就会被替换为当前的模板类,成为From unordered_map To String
 */
template <typename T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
   public:
    auto operator()(const std::unordered_map<std::string, T>& config_val_unordered_map)
        -> std::string {
        YAML::Node node;
        for (auto& item : config_val_unordered_map) {
            node[item.first] = YAML::Load(LexicalCast<T, std::string>()(item.second));
        }
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

class Person {
   public:
    Person() = default;
    std::string m_name{"lwq"};
    int m_age{18};
    bool m_sex{true};

    auto ToString() const -> std::string {
        std::stringstream sstream;
        sstream << "[Person name = " << m_name << " age = " << m_age
                << " sex = " << (m_sex ? "男" : "女") << "]";
        return sstream.str();
    }
};

/*
 * 针对自定义类型Person的偏特化，当ConfigVar<T>中的T是Person时，
 * FormStr就会被替换为当前的模板类,成为From String To Person
 */
template <>
class LexicalCast<std::string, Person> {
   public:
    auto operator()(const std::string& config_str_person) -> Person {
        YAML::Node node = YAML::Load(config_str_person);
        Person person;
        person.m_name = node["name"].as<std::string>();
        person.m_age = node["age"].as<int>();
        person.m_sex = node["sex"].as<bool>();
        return person;
    }
};

/*
 * 针对自定义类型Person的偏特化，当ConfigVar<T>中的T是Person 时，
 * ToStr就会被替换为当前的模板类,成为From Person To String
 */
template <>
class LexicalCast<Person, std::string> {
   public:
    auto operator()(const Person& config_val_person) -> std::string {
        YAML::Node node;
        node["name"] = config_val_person.m_name;
        node["age"] = config_val_person.m_age;
        node["sex"] = config_val_person.m_sex;
        std::stringstream sstream;
        sstream << node;
        return sstream.str();
    }
};

}  // namespace wtsclwq