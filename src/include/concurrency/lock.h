/*
 * @Description:
 * @LastEditTime: 2023-03-21 14:07:28
 */
#pragma once

#include <atomic>
#include <mutex>
#include <shared_mutex>
namespace wtsclwq {

class SpinLock {
  public:
    SpinLock() = default;
    SpinLock(const SpinLock&) = delete;
    SpinLock(SpinLock&&) = delete;
    auto operator=(const SpinLock&) -> SpinLock& = delete;
    auto operator=(SpinLock&&) -> SpinLock& = delete;
    ~SpinLock() = default;
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
        }
    }
    void unlock() { flag_.clear(std::memory_order_release); }

  private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
};

template <typename T>
class ScopedLock {
  public:
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock(ScopedLock&&) = delete;
    auto operator=(const ScopedLock&) -> ScopedLock& = delete;
    auto operator=(ScopedLock&&) -> ScopedLock& = delete;

    explicit ScopedLock(T& lock) : lock_(lock) { lock_.lock(); }
    ~ScopedLock() { lock_.unlock(); }

  private:
    T& lock_;
};

class RWLock {
  public:
    void ReadLock() { mutex_.lock_shared(); }
    void ReadUnlock() { mutex_.unlock_shared(); }
    void WriteLock() { mutex_.lock(); }
    void WriteUnlock() { mutex_.unlock(); }

  private:
    std::shared_mutex mutex_;
};

class ScopedReadLock {
  public:
    ScopedReadLock(const ScopedReadLock&) = delete;
    ScopedReadLock(ScopedReadLock&&) = delete;
    auto operator=(const ScopedReadLock&) -> ScopedReadLock& = delete;
    auto operator=(ScopedReadLock&&) -> ScopedReadLock& = delete;

    explicit ScopedReadLock(RWLock& rwlock) : rwlock_(rwlock) {
        rwlock_.ReadLock();
    }
    ~ScopedReadLock() { rwlock_.ReadUnlock(); }

  private:
    RWLock& rwlock_;
};

class ScopedWriteLock {
  public:
    ScopedWriteLock(const ScopedWriteLock&) = delete;
    ScopedWriteLock(ScopedWriteLock&&) = delete;
    auto operator=(const ScopedWriteLock&) -> ScopedWriteLock& = delete;
    auto operator=(ScopedWriteLock&&) -> ScopedWriteLock& = delete;

    explicit ScopedWriteLock(RWLock& rwlock) : rwlock_(rwlock) {
        rwlock_.WriteLock();
    }
    ~ScopedWriteLock() { rwlock_.WriteUnlock(); }

  private:
    RWLock& rwlock_;
};

}  // namespace wtsclwq
