#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"
/*
 * @Description:
 * @LastEditTime: 2023-03-28 21:18:52
 */
#include "../include/io/io_manager.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "../include/log/log_manager.h"
#include "../include/util/macro.h"

namespace wtsclwq {
static Logger::ptr sys_logger{GET_LOGGER_BY_NAME("system")};

auto IOManager::FdContext::GetEventHandler(EventType event)
    -> IOManager::FdContext::EventHandler& {
    switch (event) {
        case READ:
            return this->read_handler;
        case WRITE:
            return this->write_handler;
        default:
            WTSCLWQ_ASSERT(false, "GetEventHandler()错误，没有对应的事件");
    }
}

void IOManager::FdContext::ResetEventHandler(EventHandler& handler) {
    handler.scheduler = nullptr;
    handler.callback = nullptr;
    handler.fiber.reset();
}

void IOManager::FdContext::TriggerEvent(EventType event) {
    WTSCLWQ_ASSERT(this->events & event, "事件必须存在");
    events = static_cast<EventType>(events & static_cast<EventType>(~event));
    EventHandler& handler = GetEventHandler(event);
    if (handler.callback != nullptr) {
        handler.scheduler->Schedule(handler.callback);
    } else {
        handler.scheduler->Schedule(handler.fiber);
    }
    ResetEventHandler(handler);
}

IOManager::IOManager(size_t thread_num, bool use_caller, std::string name)
    : Scheduler(thread_num, use_caller, std::move(name)),
      m_epfd(epoll_create1(EPOLL_CLOEXEC)) {
    WTSCLWQ_ASSERT(m_epfd > 0, "epoll_create() error");
    int flag;
    flag = pipe2(m_tickle_fds, O_CLOEXEC);
    WTSCLWQ_ASSERT(flag != -1, "pipe2() error");

    epoll_event ep_event{};
    ep_event.events = EPOLLIN | EPOLLET;  // 读事件 | 边缘触发
    ep_event.data.fd = m_tickle_fds[0];

    // 将m_tickle_fds[0]存的fd设为非阻塞模式
    flag = fcntl(m_tickle_fds[0], F_SETFL, O_NONBLOCK);
    WTSCLWQ_ASSERT(flag != -1, "fcntl() error");

    flag = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickle_fds[0], &ep_event);
    WTSCLWQ_ASSERT(flag != -1, "epoll_ctl() error");

    const size_t context_vec_size = 64;
    ContextVecResize(context_vec_size);

    this->Start();
}
IOManager::~IOManager() {
    this->Stop();
    close(m_epfd);
    close(m_tickle_fds[0]);
    close(m_tickle_fds[1]);
}

auto IOManager::AddEvent(int filedsc, EventType new_event,
                         std::function<void()> callback) -> int {
    m_rwlock.ReadLock();
    FdContext::ptr fd_ctx{};
    if (filedsc < m_fd_contexts_vec.size()) {
        fd_ctx = m_fd_contexts_vec[filedsc];
        m_rwlock.ReadUnlock();
    } else {
        m_rwlock.ReadUnlock();
        m_rwlock.WriteLock();
        ContextVecResize(filedsc * 1.5);
        fd_ctx = m_fd_contexts_vec[filedsc];
        m_rwlock.WriteUnlock();
    }

    // 锁住fd_ctx
    ScopedLock<FdContext::MutexType> fd_lock(fd_ctx->mutex);
    if ((fd_ctx->events & new_event) != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "AddEvent()错误:向同一fd重复注册事件 feledsc = %d, "
                         "new_event = %d, fd_ctx.events = %d",
                         filedsc, new_event, fd_ctx->events);
        return -1;
    }
    // 如果ctx中的事件为空，说明未注册过，使用ADD模式，否则使用MOD模式
    int op_type = (fd_ctx->events == NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    epoll_event ep_event{};
    // 旧事件 | 欲添加事件 设为边缘出发
    ep_event.events = EPOLLET | fd_ctx->events | new_event;  // NOLINT
    // 自封装fd_ctx指针到ep_event当中，便于epoll_wait()在监听到事件后，处理事件时使用fd_ctx的方法和属性
    ep_event.data.ptr = fd_ctx.get();
    // 注册对filedesc上的ep_event监听
    int flag = epoll_ctl(m_epfd, op_type, filedsc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl()错误, m_epfd = %d, op_type = %d, filedesc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedsc, fd_ctx->events, errno);
        return -1;
    }
    // 待执行的IO事件数+1
    ++m_pending_event_count;

    fd_ctx->events = static_cast<EventType>(fd_ctx->events | new_event);
    FdContext::EventHandler& event_handler = fd_ctx->GetEventHandler(new_event);
    WTSCLWQ_ASSERT(event_handler.scheduler == nullptr &&
                       event_handler.fiber == nullptr &&
                       event_handler.callback == nullptr,
                   "event_ctx非空");
    event_handler.scheduler = Scheduler::GetThisThreadScheduler();
    if (callback != nullptr) {
        event_handler.callback.swap(callback);
    } else {
        event_handler.fiber = Fiber::GetCurFiber();
        WTSCLWQ_ASSERT(event_handler.fiber->GetState() == Fiber::EXEC,
                       "event_cxt.fiber is not running");
    }
    return 0;
}

[[maybe_unused]] auto IOManager::DelEvent(int filedesc, EventType event)
    -> bool {
    m_rwlock.ReadLock();
    if (filedesc >= m_fd_contexts_vec.size()) {
        return false;
    }
    FdContext::ptr fd_ctx{m_fd_contexts_vec[filedesc]};
    m_rwlock.ReadUnlock();

    ScopedLock<FdContext::MutexType> fd_lock(fd_ctx->mutex);
    // 如果fx_ctx中要删除的事件未设置过，直接return
    if ((fd_ctx->events & event) == 0) {
        return false;
    }
    // 移除event之后的events
    auto new_events =
        static_cast<EventType>(fd_ctx->events & static_cast<EventType>(~event));
    // 如果新的events成为0了，说明epoll不需要再监听来，直接DEL即可，否则MOD
    int op_type = (new_events != NONE) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event{};
    ep_event.events = EPOLLET | new_events;  // NOLINT
    ep_event.data.ptr = fd_ctx.get();
    int flag = epoll_ctl(m_epfd, op_type, filedesc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl()错误, m_epfd = %d, op_type = %d, filedesc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedesc, fd_ctx->events, errno);
        return false;
    }
    // 待执行的事件数-1
    --m_pending_event_count;
    // 更新fd_ctx的事件
    fd_ctx->events = new_events;
    // 重置fd_ctx欲删除事件的handler
    FdContext::EventHandler& event_handler = fd_ctx->GetEventHandler(event);
    fd_ctx->ResetEventHandler(event_handler);
    return true;
}

auto IOManager::CancelEvent(int filedesc, EventType event) -> bool {
    m_rwlock.ReadLock();
    if (filedesc >= m_fd_contexts_vec.size()) {
        return false;
    }
    FdContext::ptr fd_ctx{m_fd_contexts_vec[filedesc]};
    m_rwlock.ReadUnlock();

    ScopedLock<FdContext::MutexType> fd_lock(fd_ctx->mutex);
    // 如果fx_ctx中要删除的事件未设置过，直接return
    if ((fd_ctx->events & event) == 0) {
        return false;
    }
    auto new_events =
        static_cast<EventType>(fd_ctx->events & static_cast<EventType>(~event));
    // 如果新的events成为0了，说明epoll不需要再监听来，直接DEL即可，否则MOD
    int op_type = (new_events != NONE) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
    epoll_event ep_event{};
    ep_event.events = EPOLLIN | new_events;
    ep_event.data.ptr = fd_ctx.get();
    int flag = epoll_ctl(m_epfd, op_type, filedesc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl error, m_epfd = %d, op_type = %d, filedesc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedesc, fd_ctx->events, errno);
        return false;
    }
    // 删除事件监听会触发事件回调
    fd_ctx->TriggerEvent(event);
    // 待执行的事件数-1
    --m_pending_event_count;
    return true;
}

auto IOManager::CancelAll(int filedesc) -> bool {
    m_rwlock.ReadLock();
    if (filedesc >= m_fd_contexts_vec.size()) {
        return false;
    }
    FdContext::ptr fd_ctx{m_fd_contexts_vec[filedesc]};
    m_rwlock.ReadUnlock();

    ScopedLock<FdContext::MutexType> fd_lock(fd_ctx->mutex);
    // 如果fx_ctx中要删除的事件未设置过，直接return
    if ((fd_ctx->events) == 0) {
        return false;
    }
    // 如果新的events成为0了，说明epoll不需要再监听来，直接DEL即可，否则MOD
    int op_type = EPOLL_CTL_DEL;
    epoll_event ep_event{};
    ep_event.events = 0;  // NOLINT
    ep_event.data.ptr = fd_ctx.get();

    int flag = epoll_ctl(m_epfd, op_type, filedesc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl error, m_epfd = %d, op_type = %d, filedesc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedesc, fd_ctx->events, errno);
        return false;
    }
    // 触发读写事件
    if ((fd_ctx->events & READ) != NONE) {
        fd_ctx->TriggerEvent(READ);
        --m_pending_event_count;
    }
    if ((fd_ctx->events & WRITE) != NONE) {
        fd_ctx->TriggerEvent(WRITE);
        --m_pending_event_count;
    }
    WTSCLWQ_ASSERT(fd_ctx->events == NONE, "CancelAll error");
    return true;
}

auto IOManager::GetThisThreadIOManager() -> IOManager* {
    return dynamic_cast<IOManager*>(Scheduler::GetThisThreadScheduler());
}

void IOManager::ContextVecResize(size_t size) {
    size_t old_size = m_fd_contexts_vec.size();
    m_fd_contexts_vec.resize(size);
    for (size_t i = old_size; i < m_fd_contexts_vec.size(); ++i) {
        m_fd_contexts_vec[i].reset(new FdContext());
        // 每个进程的fd都是从0开始分配的，其中0,1,2被占用
        m_fd_contexts_vec[i]->filedesc = static_cast<int>(i);
    }
}

void IOManager::Tickle() {
    if (!HasIdleThread()) {
        return;
    }
    size_t write_size = write(m_tickle_fds[1], "T", 1);
    WTSCLWQ_ASSERT(write_size == 1, "write() error");
}

auto IOManager::OnStop() -> bool {
    uint64_t timeout{0};
    return OnStop(timeout);
}

auto IOManager::OnStop(uint64_t& timeout) -> bool {
    timeout = GetNextTimer();
    return timeout == UINT64_MAX && m_pending_event_count == 0 &&
           Scheduler::OnStop();
}

void IOManager::OnIdle() {
    const int events_num{64};
    auto event_list_ptr{std::make_unique<epoll_event[]>(events_num)};
    while (true) {
        uint64_t next_timeout = 0;
        if (OnStop(next_timeout)) {
            if (next_timeout == UINT64_MAX) {
                break;
            }
        }
        int nums;
        while (true) {
            static const uint64_t max_timeout = 5000;
            static const int max_events = 64;
            if (next_timeout != UINT64_MAX) {
                next_timeout = std::min(max_timeout, next_timeout);
            } else {
                next_timeout = max_timeout;
            }
            nums = epoll_wait(m_epfd, event_list_ptr.get(), max_events,
                              static_cast<int>(next_timeout));
            if (nums >= 0) {
                break;
            }
        }
        // 处理定时器
        // !利用了epoll_wait的副作用：
        // 1.先利用GetNextTimer获得距离执行下一个定时任务的剩余时间[next_timeout]
        // 2.epoll_wait等待这段时间[next_timeout]
        // 3.然后取出所有的超时定时器的回调（正好到达指定时间的也算作超时）
        // 4.把这些回调作为任务用调度器执行
        std::vector<std::function<void()>> function_vec{};
        ListExpiredCallbacks(function_vec);
        if (!function_vec.empty()) {
            Schedule(function_vec.begin(), function_vec.end());
            function_vec.clear();
        }

        for (int i = 0; i < nums; ++i) {
            epoll_event& ep_event = event_list_ptr[i];
            if (ep_event.data.fd == m_tickle_fds[0]) {
                uint8_t dummy[256];
                while (read(m_tickle_fds[0], dummy, sizeof(dummy)) > 0) {
                }
                continue;
            }
            auto* fd_ctx = static_cast<FdContext*>(ep_event.data.ptr);
            ScopedLock<FdContext::MutexType> lock(fd_ctx->mutex);
            // 存疑该事件的fd出现错误或者失效,则直接注册读+写事件，并且触发，
            // 否则有可能出现注册的事件永远执行不到的情况
            if ((ep_event.events & (EPOLLERR | EPOLLHUP)) != NONE) {
                ep_event.events |= (EPOLLIN | EPOLLOUT) & fd_ctx->events;
            }

            uint32_t real_events = NONE;
            // 如果有读事件
            if ((ep_event.events & EPOLLIN) != NONE) {
                real_events |= READ;
            }
            // 如果有写事件
            if ((ep_event.events & EPOLLOUT) != NONE) {
                real_events |= WRITE;
            }
            // 如果事件都已经被触发并处理
            if ((fd_ctx->events & real_events) == NONE) {
                continue;
            }
            // real_events是本次要处理的事件，因此需要在fd_ctx中移除
            uint32_t left_events = (fd_ctx->events & ~real_events);
            int op_type = (left_events != NONE) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
            ep_event.events = EPOLLET | left_events;

            int flag = epoll_ctl(m_epfd, op_type, fd_ctx->filedesc, &ep_event);
            if (flag == -1) {
                LOG_CUSTOM_ERROR(
                    sys_logger,
                    "epoll_ctl()错误, m_epfd = %d, op_type = %d, filedesc "
                    "= %d, fd_ctx.events = %d, errno = %d",
                    m_epfd, op_type, fd_ctx->filedesc, fd_ctx->events, errno);
                continue;
            }
            if ((real_events & READ) != NONE) {
                fd_ctx->TriggerEvent(READ);
                --m_pending_event_count;
            }
            if ((real_events & WRITE) != NONE) {
                fd_ctx->TriggerEvent(WRITE);
                --m_pending_event_count;
            }
        }
        Fiber::ptr cur = Fiber::GetCurFiber();
        auto* raw_ptr = cur.get();
        cur.reset();  // 防止换出后无法正常析构
        raw_ptr->Yield();
    }
}

void IOManager::OnTimerInsertedFront() { Tickle(); }

}  // namespace wtsclwq
#pragma clang diagnostic pop