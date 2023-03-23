/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-17 23:50:28
 * @LastEditTime: 2023-03-23 22:40:33
 */
#pragma once

#include <sys/types.h>
#include <sys/ucontext.h>
#include <ucontext.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>

namespace wtsclwq {
const int FIBER_STACK_SIZE = 1024 * 1024;

class Fiber : public std::enable_shared_from_this<Fiber> {
  public:
    using ptr = std::shared_ptr<Fiber>;

    enum State { INIT, HOLD, EXEC, TERM, READY, EXCEPT };

  private:
    /**
     * @description:
     * 每个线程中*第一个*协程的构造函数，也就是主协程。
     * 主协程的ID必定为0,且不需要新建栈空间，运行状态默认为EXEC
     */
    Fiber();

  public:
    Fiber(const Fiber &) = delete;
    Fiber(Fiber &&) = delete;
    auto operator=(const Fiber &) -> Fiber & = delete;
    auto operator=(Fiber &&) -> Fiber & = delete;

    /**
     * @description:
     * 构造子协程,子协程在@stack_size大小的栈空间上执行@call_back函数，子协程ID是一个原子自增的
     * @param {function<void()>} call_back 新协程要执行的函数
     * @param {size_t} stack_size 协程栈大小
     */
    explicit Fiber(std::function<void()> call_back, size_t stack_size = 0);

    /**
     * @description: 析构函数，分类判断：
     * 1.子协程：回收栈空间
     * 2.主协程：将cur_fiber置为nullptr，当前线程中没有需要运行的协程了
     */
    ~Fiber();

    /**
     * @description: 重置协程的回调函数为@call_back，重置状态，重复利用栈空间
     * @param {function<void()>} call_back 新的回调函数
     */
    void Reset(std::function<void()> call_back);

    /**
     * @description: 将this协程上升为cur协程，并与main协程互换上下文。
     * 注意：当前协程模型只允许从主协程切换到子协程执行，
     *      因此子协程对象执行SwapIn()时，cur协程必定为主协程
     * @return {*}
     */
    void SwapIn();

    void SwapInFromScheduler();

    /**
     * @description: 将this协程切换到后台，让主协程重回cur协程
     * @return {*}
     */
    void SwapOut();

    void SwapOutBackScheduler();

    /**
     * @description: 设置正在运行的协程为@param
     * @return {*}
     * @param {Fiber} *fiber
     */

    auto GetId() const -> uint64_t;

    auto GetState() const -> Fiber::State;

    void SetState(Fiber::State state);

    auto IsFinish() const noexcept -> bool;

    /* ************************************************************** */
    /* ************************************************************** */
    /* ************************************************************** */

    static void SetCurFiber(Fiber *fiber);

    /**
     * @description: 获取当前协程，如果没有则新建主协程作为当前协程
     * @return {Fiber::ptr} t_cur_fiber or new main fiber
     */
    static auto GetCurFiber() -> Fiber::ptr;

    /**
     * @description: 将当前协程设置为Ready,切换回主协程
     */
    static void YieldToReady();

    /**
     * @description: 将当前协程设置为Hold,切换回主协程
     */
    static void YieldToHold();

    /**
     * @description: 获取当前协程ID
     * @return {uint64_t} this->m_id
     */
    static auto GetCurFiberId() -> uint64_t;

    /**
     * @description: 获取当前线程内的总协程数
     * @return {uint64_t} 协程总数
     */
    static auto TotalFibers() -> uint64_t;

    /**
     * @description: this协程要执行的方法
     */
    static void MainFunc();

  private:
    uint64_t m_id;                      // 协程ID
    uint32_t m_stack_size;              // 协程栈空间大小
    void *m_stack;                      // 协程栈空间指针
    State m_state;                      // 协程运行状态
    ucontext_t m_ctx{};                 // 用户态上下文
    std::function<void()> m_call_back;  // 回调函数
};
}  // namespace wtsclwq