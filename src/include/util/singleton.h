/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-05 22:27:34
 * @LastEditTime: 2023-03-12 13:34:58
 */
#pragma once

#include <memory>
namespace wtsclwq {

template <typename T>
class Singleton final {
   public:
    // 可变参数模板，适配有参构造和无参构造
    template <typename... Args>
    static auto GetInstance(Args&&... args) -> T* {
        // 使用完美转发，无论args是左值还是右值都可以正确传递
        static T instance{std::forward<Args>(args)...};
        return &instance;
    }
};

template <typename T>
class SingletonPtr final {
   public:
    // 可变参数模板，适配有参构造和无参构造
    template <typename... Args>
    static auto GetInstancePtr(Args&&... args) -> std::shared_ptr<T> {
        // 使用完美转发，无论args是左值还是右值都可以正确传递
        static auto instance = std::make_shared<T>(std::forward<Args>(args)...);
        return instance;
    }
};

}  // namespace wtsclwq