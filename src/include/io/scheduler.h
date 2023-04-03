/*
 * @Description:
 * @LastEditTime: 2023-03-27 22:41:46
 */
#pragma once

#include <sched.h>

#include <atomic>
#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "../concurrency/fiber.h"
#include "../concurrency/lock.h"
#include "../concurrency/thread.h"
namespace wtsclwq {
class Scheduler {
  public:
    friend class wtsclwq::Fiber;
    Scheduler(const Scheduler &) = delete;
    Scheduler(Scheduler &&) = delete;
    auto operator=(const Scheduler &) -> Scheduler & = delete;
    auto operator=(Scheduler &&) -> Scheduler & = delete;

    using ptr = std::shared_ptr<Scheduler>;
    using MutexType = std::mutex;
    /**
     * @description:
     * @param {size_t} thread_num 调度器初始的线程数目
     * @param {bool} use_caller 调度器创建者线程是否加入调度器
     * @param {string} name 调度器名称
     * @return {*}
     */
    explicit Scheduler(size_t thread_num = 1, bool use_caller = true,
                       std::string name = "");

    virtual ~Scheduler();

    [[nodiscard]] auto GetName() const -> std::string;

    [[nodiscard]] auto GetActiveThreadCount() const -> size_t;

    void SetActiveThreadCount(size_t count);

    [[nodiscard]] auto IsHasIdleThread() const -> bool;

    void SetIdleThreadCount(size_t count);

    [[nodiscard]] auto IsAutoStoping() const -> bool;

    void SetIsAutoStoping(bool is_auto_stop);

    [[nodiscard]] auto GetRootThreadId() const -> pid_t;

    void SetRootThreadId(pid_t thread_id);

    [[nodiscard]] auto GetMutex() const -> MutexType &;

    /**
     * @description: 获取当前线程内调度器指针
     * @return {*}
     */
    static auto GetThisThreadScheduler() -> Scheduler *;

    /**
     * @description: 并不是线程的主协程,而是负责调度工作的主工作协程
     * @return {*}
     */
    static auto GetScheduleFiber() -> Fiber *;

    /**
     * @description: 启动调度器(线程池)
     * @return {*}
     */
    void Start();

    /**
     * @description: 关闭调度器(线程池)
     * @return {*}
     */
    void Stop();

    /**
     * @description: 调度器是否有空闲的线程
     * @return {*}
     */
    [[nodiscard]] auto HasIdleThread() const -> bool;

    /**
     * @description: 添加任务 thread-safe
     * @param {Executable} & 模板对象,可以是fiber和fubction,用来构建Task
     * @param {pid_t} thread_id 任务要绑定到的线程id
     * @param {bool} is_priority 是否优先调度
     */
    template <typename Executable>
    void Schedule(Executable &&exec, pid_t thread_id = -1,
                  bool is_priority = false);

    /**
     * @description: 添加多个任务thread-safe
     * @param{InputIterator} 迭代器起点
     * @param{InputIterator} 迭代器终点
     */
    template <typename InputIterator>
    void Schedule(InputIterator begin, InputIterator end);

    /**
     * @description: 调度器需要唤醒其他线程
     * @return {*}
     */
    virtual void Tickle();

    /**
     * @description: 调度器停止时的回调函数,做一些收尾工作,判断是否可以停止
     * @return {bool} 是否停止成功
     */
    virtual auto OnStop() -> bool;

    /**
     * @description: 调度器空闲(有空闲线程)时的回调函数
     */
    virtual void OnIdle();

  private:
    /**
     * @description: 等待分配给线程执行的任务,肯能是fiber或者function
     * !重载多个构造函数,搭配模板使用,根据传入参数的不同,可以构建不同的任务对象
     */
    struct Task {
      public:
        Task(const Task &) = default;
        auto operator=(const Task &) -> Task & = default;
        Task(Task &&) = delete;
        auto operator=(Task &&) -> Task & = delete;

        using ptr = std::shared_ptr<Task>;

        Task() = default;

        Task(const Fiber::ptr &fib, pid_t tid) : fiber(fib), thread_id(tid) {}

        Task(Fiber::ptr &&fib, pid_t tid)
            : fiber(std::move(fib)), thread_id(tid) {}

        Task(const std::function<void()> &function, pid_t tid)
            : func(function), thread_id(tid) {}

        Task(std::function<void()> &&function, pid_t tid)
            : func(std::move(function)), thread_id(tid) {}

        ~Task() = default;

        void Reset() {
            fiber = nullptr;
            func = nullptr;
            thread_id = -1;
        }

        Fiber::ptr fiber{};
        std::function<void()> func{};
        pid_t thread_id{-1};
    };

    /**
     * @description:添加任务 non-thread-safe
     * @param {Executable} & 模板对象,可以是fiber和function,用来构建task
     * @param {pid_t} thread_id 任务将要绑定的线程id
     * @param {bool} is_priority 时候优先调度
     * @return {bool} 是否是空闲状态下的第一个新任务
     */
    template <typename Executable>
    auto ScheduleNoLock(Executable &&exec, pid_t thread_id,
                        bool is_priority = false) -> bool;

    /**
     * @description: 调度器的工作方法
     */
    void Run();

    std::string m_name{};                         // 调度器名称
    std::vector<Thread::ptr> m_threads_vec{};     // 线程池
    std::list<Task::ptr> m_task_list{};           // 任务队列
    std::vector<int> m_thread_id_vec{};           // 线程id集合
    Fiber::ptr m_root_fiber{};                    // 调度器的主工作协程
    size_t m_thread_count{0};                     // 线程池总线程数
    std::atomic_size_t m_active_thread_count{0};  // 活跃线程数
    std::atomic_size_t m_idle_thread_count{0};    // 空闲线程数
    bool m_is_stop{true};                         // 是否处于停止状态
    bool m_is_auto_stop{false};                   // 是否自动停止
    int m_root_thread_id{0};                      // 调度器创建者线程id
    mutable MutexType m_mutex{};
};

template <typename Executable>
void Scheduler::Schedule(Executable &&exec, pid_t thread_id, bool is_priority) {
    bool need_tickle{false};
    {
        ScopedLock<MutexType> lock(m_mutex);
        need_tickle = ScheduleNoLock(std::forward<Executable>(exec), thread_id,
                                     is_priority);
    }
    // 任务队列从0到1,通知外部有任务来了,需要工作
    if (need_tickle) {
        Tickle();
    }
}

template <typename InputIterator>
void Scheduler::Schedule(InputIterator begin, InputIterator end) {
    bool need_tickle{false};
    {
        ScopedLock<MutexType> lock(m_mutex);
        while (begin != end) {
            need_tickle = ScheduleNoLock(*begin, -1) || need_tickle;
            ++begin;
        }
    }
    if (need_tickle) {
        Tickle();
    }
}

template <typename Executable>
auto Scheduler::ScheduleNoLock(Executable &&exec, pid_t thread_id,
                               bool is_priority) -> bool {
    // 任务队列是否为空
    bool need_tickle{m_task_list.empty()};
    //! 注意完美转发的用法
    Task::ptr task{
        std::make_shared<Task>(std::forward<Executable>(exec), thread_id)};
    if (task->fiber || task->func) {  // 确保任务填充成功
        if (is_priority) {
            m_task_list.push_front(std::move(task));  // move(shared_ptr)
        } else {
            m_task_list.push_back(std::move(task));   // move(shared_ptr)
        }
    }
    return need_tickle;
}
}  // namespace wtsclwq