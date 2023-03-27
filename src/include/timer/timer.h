/*
 * @Description:
 * @LastEditTime: 2023-03-28 00:58:39
 */
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "../concurrency/lock.h"
#include "../concurrency/thread.h"

namespace wtsclwq {
class TimerManager;
class Timer : public std::enable_shared_from_this<Timer> {
    friend class TimerManager;

  public:
    using ptr = std::shared_ptr<Timer>;

    auto Cancel() -> bool;
    auto Refresh() -> bool;
    auto Reset(uint64_t msecend, bool from_now) -> bool;

  private:
    explicit Timer(uint64_t next);
    Timer(uint64_t msecond, std::function<void()> callback, bool is_recur,
          TimerManager *manager);
    struct Comparator {
        auto operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const
            -> bool;
    };

    uint64_t m_ms{0};                    // 执行周期
    uint64_t m_next{0};                  // 执行的绝对时间戳
    std::function<void()> m_callback{};  // 回调函数
    bool m_is_recur{false};              // 是否重复执行
    TimerManager *m_manager{nullptr};
};

class TimerManager {
    friend class Timer;

  public:
    TimerManager(const TimerManager &) = delete;
    TimerManager(TimerManager &&) = delete;
    auto operator=(const TimerManager &) -> TimerManager & = delete;
    auto operator=(TimerManager &&) -> TimerManager & = delete;

    TimerManager();
    virtual ~TimerManager() = default;

    auto AddTimer(uint64_t msecend, std::function<void()> callback,
                  bool is_recur = false) -> Timer::ptr;

    auto AddCondictionTimer(uint64_t msecend,
                            const std::function<void()> &callback,
                            const std::weak_ptr<void> &weak_cond,
                            bool is_recur = false) -> Timer::ptr;

    /**
     * @description: 返回[距离]执行下一个任务的时间
     * @return {*}
     */
    auto GetNextTimer() -> uint64_t;

    auto HasTimer() -> bool;

    void ListExpiredCallbacks(
        std::vector<std::function<void()>> &callbacks_vec);

  private:
    /**
     * @description:
     * 如果有新加的定时任务处于队列的最前方，那么需要通知一下调度器，之前的wait时间可能失效
     * @return {*}
     */
    virtual void OnTimerInsertedFront() = 0;
    auto DetectClockRollover(uint64_t now_ms) -> bool;

    uint64_t m_previous_exe_time{0};
    std::set<Timer::ptr, Timer::Comparator> m_timers_set{};
    RWLock m_rw_lock{};
};
}  // namespace wtsclwq