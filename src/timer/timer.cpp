/*
 * @Description:
 * @LastEditTime: 2023-03-28 01:00:14
 */
#include "../include/timer/timer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

#include "../include/util/time_util.h"

namespace wtsclwq {

auto Timer::Comparator::operator()(const Timer::ptr &lhs,
                                   const Timer::ptr &rhs) const -> bool {
    if (!lhs && !rhs) {
        return false;
    }
    if (!lhs) {
        return true;
    }
    if (!rhs) {
        return false;
    }
    if (lhs->m_next < rhs->m_next) {
        return true;
    }
    if (lhs->m_next > rhs->m_next) {
        return false;
    }
    return lhs.get() < rhs.get();
}
Timer::Timer(uint64_t next) : m_next(next) {}

Timer::Timer(uint64_t msecond, std::function<void()> callback, bool is_recur,
             TimerManager *manager)
    : m_ms(msecond), m_next(wtsclwq::GetCurrentMS() + m_ms),
      m_callback(std::move(callback)), m_is_recur(is_recur),
      m_manager(manager) {}

auto Timer::Cancel() -> bool {
    ScopedWriteLock lock(m_manager->m_rw_lock);
    if (m_callback) {
        m_callback = nullptr;
        auto iter = m_manager->m_timers_set.find(shared_from_this());
        m_manager->m_timers_set.erase(iter);
        return true;
    }
    return false;
}

auto Timer::Refresh() -> bool {
    ScopedWriteLock lock(m_manager->m_rw_lock);
    if (!m_callback) {
        return false;
    }
    auto iter = m_manager->m_timers_set.find(shared_from_this());
    if (iter == m_manager->m_timers_set.end()) {
        return false;
    }
    // 为什么移除在插入，因为set的比较逻辑是timer的m_ms，如果直接修改了m_ms会影响数据结构的排序正确性
    m_manager->m_timers_set.erase(iter);
    m_next = wtsclwq::GetCurrentMS() + m_ms;
    m_manager->m_timers_set.insert(shared_from_this());
    return true;
}

auto Timer::Reset(uint64_t msecend, bool from_now) -> bool {
    m_manager->m_rw_lock.WriteLock();
    if (msecend == m_ms && !from_now) {
        return true;
    }
    if (!m_callback) {
        return false;
    }

    auto iter = m_manager->m_timers_set.find(shared_from_this());
    if (iter == m_manager->m_timers_set.end()) {
        return false;
    }
    // 为什么移除在插入，因为set的比较逻辑是timer的m_ms，如果直接修改了m_ms会影响数据结构的排序正确性
    m_manager->m_timers_set.erase(iter);
    uint64_t start = 0;
    if (from_now) {
        start = wtsclwq::GetCurrentMS();
    } else {
        // 比如原先m_ms = 5, m_next = 5,就可以取得
        // start = 0，也就是它上一次执行过的时间(对于循环计时器)
        start = m_next - m_ms;
    }
    m_ms = msecend;
    m_next = start + m_ms;
    iter = m_manager->m_timers_set.insert(shared_from_this()).first;
    bool at_front = (iter == m_manager->m_timers_set.begin());
    m_manager->m_rw_lock.WriteUnlock();
    if (at_front) {
        m_manager->OnTimerInsertedFront();
    }
    return true;
}
TimerManager::TimerManager() : m_previous_exe_time(wtsclwq::GetCurrentMS()) {}

auto TimerManager::AddTimer(uint64_t msecend, std::function<void()> callback,
                            bool is_recur) -> Timer::ptr {
    Timer::ptr timer(new Timer(msecend, std::move(callback), is_recur, this));
    m_rw_lock.WriteLock();
    auto iter{m_timers_set.insert(timer).first};
    bool need_tickle{iter == m_timers_set.begin()};
    m_rw_lock.WriteUnlock();
    if (need_tickle) {
        OnTimerInsertedFront();
    }
    return timer;
}

static void OnTimer(const std::weak_ptr<void> &weak_cond,
                    const std::function<void()> &callback) {
    std::shared_ptr<void> tmp = weak_cond.lock();
    if (tmp != nullptr) {
        callback();
    }
}

auto TimerManager::AddCondictionTimer(uint64_t msecend,
                                      const std::function<void()> &callback,
                                      const std::weak_ptr<void> &weak_cond,
                                      bool is_recur) -> Timer::ptr {
    return AddTimer(
        msecend, [weak_cond, callback] { return OnTimer(weak_cond, callback); },
        is_recur);
}

auto TimerManager::GetNextTimer() -> uint64_t {
    ScopedReadLock lock(m_rw_lock);
    if (m_timers_set.empty()) {
        return ~0ULL;
    }
    const Timer::ptr &next = *m_timers_set.begin();
    uint64_t now_ms = wtsclwq::GetCurrentMS();
    if (now_ms >= next->m_next) {
        return 0;
    }
    return next->m_next - now_ms;
}

auto TimerManager::HasTimer() -> bool {
    ScopedReadLock lock(m_rw_lock);
    return !m_timers_set.empty();
}

void TimerManager::ListExpiredCallbacks(
    std::vector<std::function<void()>> &callbacks_vec) {
    uint64_t now_ms = wtsclwq::GetCurrentMS();
    std::vector<Timer::ptr> expired_vec;
    {
        ScopedReadLock read_lock(m_rw_lock);
        if (m_timers_set.empty()) {
            return;
        }
    }
    ScopedWriteLock write_lock(m_rw_lock);

    // 检查系统事件是否被回拨超过1小时
    bool rollover = DetectClockRollover(now_ms);
    // 系统事件正常 且 无定时器等待超时
    if (!rollover && (*m_timers_set.begin())->m_next > now_ms) {
        return;
    }

    Timer::ptr now_timer(new Timer(now_ms));

    auto iter = m_timers_set.lower_bound(now_timer);
    // 如果系统事件被回拨过，则认定所有定时器都已经超时
    if (rollover) {
        iter = m_timers_set.end();
    }
    // !m_next 大于或等于 当前事件的定时器被认定为超时
    // 因为多个timer的next_time可能会相同，
    // 而lower_bound是寻找的左边界，所以需要while一下，去除掉所有的超时timer
    while (iter != m_timers_set.end() && (*iter)->m_next == now_timer->m_next) {
        ++iter;
    }
    // 取出所有超时的定时器
    expired_vec.insert(expired_vec.begin(), m_timers_set.begin(), iter);
    m_timers_set.erase(m_timers_set.begin(), iter);
    callbacks_vec.reserve(expired_vec.size());
    for (auto &timer : expired_vec) {
        callbacks_vec.push_back(timer->m_callback);
        if (timer->m_is_recur) {
            timer->m_next = now_ms + timer->m_ms;
            // 对于循环的定时器需要重新加进去
            m_timers_set.insert(timer);
        } else {
            timer->m_callback = nullptr;
        }
    }
}

auto TimerManager::DetectClockRollover(uint64_t now_ms) -> bool {
    // 系统时间被回拨超过一个小时
    const int ROLLOVER_TIME = 60 * 60 * 1000;
    bool rollover = now_ms < m_previous_exe_time &&
                    now_ms < (m_previous_exe_time - ROLLOVER_TIME);
    m_previous_exe_time = now_ms;
    return rollover;
}
}  // namespace wtsclwq
