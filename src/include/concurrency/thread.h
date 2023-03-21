/*
 * @Description: #
 * @LastEditTime: 2023-03-21 13:26:27
 */

#pragma once

#include <pthread.h>
#include <sched.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "../concurrency/semaphore.h"
namespace wtsclwq {
class Thread {
  public:
    Thread(const Thread &) = delete;
    Thread(Thread &&) = delete;
    auto operator=(const Thread &) -> Thread & = delete;
    auto operator=(Thread &&) -> Thread & = delete;

    using ptr = std::shared_ptr<Thread>;

    Thread(std::function<void()> call_back, std::string name);
    ~Thread();

    [[nodiscard]] auto GetId() const -> pid_t;
    [[nodiscard]] auto GetName() const -> std::string;

    /**
     * @description: 等待线程执行完成
     * @return {*}
     */
    void Join();

    /**
     * @description: 获取当前线程指针
     * @return {Thread *} cur thread pointer
     */
    static auto GetCurThread() -> Thread *;

    /**
     * @description: 获取当前线程名称
     * @return {std::string} cur thread name
     */
    static auto GetCurThreadName() -> std::string;

    /**
     * @description: 设置当前线程名称
     * @param {string} name
     */
    void static SetCurThreadName(std::string name);

  private:
    /**
     * @description:
     * 线程的工作函数，为什么不直接运行m_call_back？因为还需要在Run内做其他的工作
     * @return {void *} 任意返回值
     * @param {void} *arg 其实一般传递this指针
     */
    static auto Run(void *arg) -> void *;

    pid_t m_id = -1;                    // OS分配的线程ID
    std::string m_name{};               // 线程名称
    pthread_t m_thread = 0;             // pthread库分配的线程ID
    std::function<void()> m_call_back;  // 线程要执行的内容
    Semaphore m_semphore;               // 自封装信号量
};
}  // namespace wtsclwq