/*
 * @Description:
 * @LastEditTime: 2023-03-25 00:45:46
 */
#pragma once
#include <array>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "fiber.h"
#include "lock.h"
#include "scheduler.h"
namespace wtsclwq {

class IOManager : public Scheduler {
  public:
    IOManager(const IOManager &) = delete;
    IOManager(IOManager &&) = delete;
    auto operator=(const IOManager &) -> IOManager & = delete;
    auto operator=(IOManager &&) -> IOManager & = delete;

    using ptr = std::shared_ptr<IOManager>;
    enum EventType { NONE = 0x0, READ = 0x1, WRITE = 0x4 };

  private:
    struct FdContext {
        using MutexType = std::mutex;
        using ptr = std::shared_ptr<FdContext>;
        struct EventHandler {
            Scheduler *scheduler{nullptr};     // 用来调度事件的scheduler
            Fiber::ptr fiber{};                // 执行事件的协程
            std::function<void()> callback{};  // 事件回调函数
        };
        auto GetEventHandler(EventType event) -> EventHandler &;
        static void ResetEventHandler(EventHandler &handler);
        void TriggerEvent(EventType event);
        int filedsc{};                 // 要监听的fd
        EventHandler read_handler{};   // 读事件的handler
        EventHandler write_handler{};  // 写事件的handler
        EventType events{NONE};        // 已经注册的事件
        MutexType mutex;
    };

  public:
    explicit IOManager(size_t thread_num = 1, bool use_caller = true,
                       std::string name = "");
    ~IOManager() override;

    /**
     * @description:
     * @return {*}1 success,2 retry, -1 error
     */
    auto AddEvent(int filedsc, EventType new_event,
                  std::function<void()> callback = nullptr) -> int;
    auto DelEvent(int filedsc, EventType event) -> bool;
    auto CancelEvent(int filedsc, EventType event) -> bool;
    auto CancleAll(int filedsc) -> bool;

    static auto GetCurIOManager() -> IOManager *;

  private:
    void Tickle() override;
    auto OnStop() -> bool override;
    void OnIdle() override;
    void ContextVecResize(size_t size);
    int m_epfd{0};                                 // epoll实例文件描述符
    int m_tickle_fds[2]{};                         // 通信管道 NOLINT
    std::atomic<size_t> m_pending_event_count{0};  // 等待执行的事件的数量
    std::vector<FdContext::ptr> m_fd_contexts_vec{};  // vec[i].fd=下标
    RWLock m_rwlock;
};

}  // namespace wtsclwq