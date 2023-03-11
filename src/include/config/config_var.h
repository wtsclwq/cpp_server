
#pragma once
#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "../log/log_manager.h"
#include "../util/cast_util.h"
#include "config_var_base.h"
/**
 * 基类是非模板的，而派生类是模板类，例如int类型的配置值、double类型的配置值...
 * 这样我们在存储配置项时使用基类，使用时再使用dynamic_pointer_cast转换其智能指针即可
 **/
namespace wtsclwq {

// FromStr仿函数，重载()运算符：T operator() (const std::string&)
// ToStr仿函数，重载()运算符： std::string operator() (const T&)
template <typename T, typename FromStr = LexicalCast<std::string, T>,
          typename ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
   public:
    using ptr = std::shared_ptr<ConfigVar<T>>;
    // 配置改变事件回调函数
    using on_change_callback =
        std::function<void(const T& old_value, const T& new_value)>;

    ConfigVar(std::string name, const T& default_value, std::string description = "")
        : ConfigVarBase(name, description), m_val(default_value) {}

    auto GetValue() const -> T { return m_val; }

    void SetValue(T val) {
        // 无变化
        if (val == m_val) {
            return;
        }
        // 如果有变化，调用所有回调函数
        for (auto& item : m_call_backs) {
            item.second(m_val, val);
        }
        m_val = val;
    }

    auto ToString() -> std::string override;

    auto FromString(std::string val) -> bool override;

    auto GetTypeName() const -> std::string override { return typeid(T).name(); }

    void AddListener(uint64_t key, on_change_callback call_back) {
        m_call_backs.insert(std::make_pair(key, call_back));
    }

    void DelListener(uint64_t key) { m_call_backs.erase(key); }

    auto GetListener(uint64_t key) -> on_change_callback {
        auto iter = m_call_backs.find(key);
        return iter != m_call_backs.end() ? iter.second : nullptr;
    }

   private:
    T m_val;
    // 变更回调函数组（std::function没有比较运算符，需要借助map来查找或者删除，key使用hash生成
    std::map<uint64_t, on_change_callback> m_call_backs;
};

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::ToString() -> std::string {
    try {
        // 尝试将value转换
        return ToStr()(m_val);
    } catch (std::exception& e) {
        std::stringstream message;
        message << e.what() << "ConfigVar<T>::ToString()，类型转换错误"
                << "convert: " << typeid(m_val).name() << "to string";
        LOG_ERROR(ROOT_LOGGER, message.str());
    }
    return "";
}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::FromString(std::string val) -> bool {
    try {
        SetValue(FromStr()(val));
    } catch (std::exception& e) {
        std::stringstream message;
        message << e.what() << "ConfigVar<T>::FromString()，类型转换错误"
                << "convert: string to " << typeid(m_val).name();
        LOG_ERROR(ROOT_LOGGER, message.str());
    }
    return false;  // ?为什么是false?
}
}  // namespace wtsclwq