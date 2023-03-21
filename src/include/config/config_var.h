/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-07 22:38:56
 * @LastEditTime: 2023-03-21 17:35:32
 */

#pragma once
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "../concurrency/lock.h"
#include "../config/config_type.h"
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

    ConfigVar(std::string name, const T& default_value,
              std::string description = "");

    auto GetValue() const -> T;

    void SetValue(T val);

    auto ToString() -> std::string override;

    auto FromString(std::string val) -> bool override;

    [[nodiscard]] auto GetTypeName() const -> std::string override;

    auto AddListener(on_change_callback call_back) -> uint64_t;

    void DelListener(uint64_t key);

    auto GetListener(uint64_t key) -> on_change_callback;

  private:
    T m_val;
    // 变更回调函数组（std::function没有比较运算符，需要借助map来查找或者删除，key使用hash生成
    std::map<uint64_t, on_change_callback> m_call_backs;

    mutable RWLock m_rwlock;
};

template <typename T, typename FromStr, typename ToStr>
ConfigVar<T, FromStr, ToStr>::ConfigVar(std::string name,
                                        const T& default_value,
                                        std::string description)
    : ConfigVarBase(name, description), m_val(default_value) {}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::GetValue() const -> T {
    ScopedReadLock lock(m_rwlock);
    return m_val;
}

template <typename T, typename FromStr, typename ToStr>
void ConfigVar<T, FromStr, ToStr>::SetValue(T val) {
    ScopedWriteLock lock(m_rwlock);
    // 无变化
    if (val == m_val) {
        return;
    }
    T old_val = m_val;
    m_val = val;
    // 如果有变化，调用所有回调函数
    for (auto& item : m_call_backs) {
        item.second(old_val, val);
    }
}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::GetTypeName() const -> std::string {
    ScopedReadLock lock(m_rwlock);
    return typeid(T).name();
}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::AddListener(on_change_callback call_back)
    -> uint64_t {
    ScopedWriteLock lock(m_rwlock);
    static uint64_t call_back_id = 0;
    m_call_backs.insert(std::make_pair(++call_back_id, call_back));
    return call_back_id;
}

template <typename T, typename FromStr, typename ToStr>
void ConfigVar<T, FromStr, ToStr>::DelListener(uint64_t key) {
    ScopedWriteLock lock(m_rwlock);
    m_call_backs.erase(key);
}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::GetListener(uint64_t key)
    -> on_change_callback {
    ScopedReadLock lock(m_rwlock);
    auto iter = m_call_backs.find(key);
    return iter != m_call_backs.end() ? iter.second : nullptr;
}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::ToString() -> std::string {
    try {
        ScopedReadLock lock(m_rwlock);
        // 尝试将value转换
        return ToStr()(m_val);
    } catch (std::exception& e) {
        std::cerr << e.what() << "ConfigVar<T>::ToString()，类型转换错误"
                  << "convert: " << typeid(m_val).name() << "to string";
    }
    return "";
}

template <typename T, typename FromStr, typename ToStr>
auto ConfigVar<T, FromStr, ToStr>::FromString(std::string val) -> bool {
    try {
        SetValue(FromStr()(val));
    } catch (std::exception& e) {
        std::cerr << e.what() << "ConfigVar<T>::FromString()，类型转换错误"
                  << "convert: string to " << typeid(m_val).name();
    }
    return false;  // ?为什么是false?
}
}  // namespace wtsclwq