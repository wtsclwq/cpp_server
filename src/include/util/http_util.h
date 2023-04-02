//
// Created by wtsclwq on 23-4-2.
//

#pragma once
#include "boost/lexical_cast.hpp"

namespace wtsclwq {
/**
 * @brief 忽略大小写比较仿函数
 */
struct CaseInsensitiveLess {
    /**
     * @brief 忽略大小写比较字符串
     */
    auto operator()(const std::string& lhs, const std::string& rhs) const
        -> bool {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

/**
 * @brief 获取Map中的key值,并转成对应类型,返回是否成功
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[out] val 保存转换后的值
 * @param[in] def 默认值
 * @return
 *      @retval true 转换成功, val 为对应的值
 *      @retval false 不存在或者转换失败 val = def
 */
template <class MapType, class T>
auto CheckGetWithDef(const MapType& m, const std::string& key, T& val,
                     const T& def = T()) -> bool {
    auto it = m.find(key);
    if (it == m.end()) {
        val = def;
        return false;
    }
    try {
        val = boost::lexical_cast<T>(it->second);
        return true;
    } catch (...) {
        val = def;
    }
    return false;
}

/**
 * @brief 获取Map中的key值,并转成对应类型
 * @param[in] m Map数据结构
 * @param[in] key 关键字
 * @param[in] def 默认值
 * @return 如果存在且转换成功返回对应的值,否则返回默认值
 */
template <class MapType, class T>
auto GetWithDef(const MapType& m, const std::string& key, const T& def = T())
    -> T {
    auto it = m.find(key);
    if (it == m.end()) {
        return def;
    }
    try {
        return boost::lexical_cast<T>(it->second);
    } catch (...) {
    }
    return def;
}

}  // namespace wtsclwq