/*
 * @Description:
 * @LastEditTime: 2023-03-25 00:55:50
 */
#include "../include/concurrency/io_manager.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>

#include "../include/log/log_manager.h"
#include "../include/util/macro.h"
namespace wtsclwq {
static Logger::ptr sys_logger{GET_LOGGER_BY_NAME("system")};

auto IOManager::FdContext::GetEventHandler(IOManager::EventType event)
    -> IOManager::FdContext::EventHandler& {
    switch (event) {
        case READ:
            return this->read_handler;
        case WRITE:
            return this->write_handler;
        default:
            WTSCLWQ_ASSERT(false, "GetEventHandler error");
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
    int flag = 0;
    flag = pipe2(m_tickle_fds, O_CLOEXEC);  // NOLINT
    WTSCLWQ_ASSERT(flag != -1, "pipe2() error");

    epoll_event ep_event{};
    ep_event.events = EPOLLIN | EPOLLET;  // 读事件 | 边缘触发
    ep_event.data.fd = m_tickle_fds[0];

    // 将m_tickle_fds[0]存的fd设为非阻塞模式
    flag = fcntl(m_tickle_fds[0], F_SETFL, O_NONBLOCK);
    WTSCLWQ_ASSERT(flag != -1, "fcntl() error");

    flag = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickle_fds[0], &ep_event);
    WTSCLWQ_ASSERT(flag != -1, "epoll_ctl() error");

    const size_t CONTEXT_VEC_SIZE = 64;
    ContextVecResize(CONTEXT_VEC_SIZE);

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
        ContextVecResize(m_fd_contexts_vec.size() * 2);
        fd_ctx = m_fd_contexts_vec[filedsc];
        m_rwlock.WriteUnlock();
    }

    // 锁住fd_ctx
    ScopedLock<FdContext::MutexType> fd_lock(fd_ctx->mutex);
    if ((fd_ctx->events & new_event) != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "AddEvent() error:向同一fd重复注册事件 feledsc = %d, "
                         "new_event = %d, fd_ctx.events = %d",
                         filedsc, new_event, fd_ctx->events);
        return -1;
    }
    // 如果ctx中的事件为空，说明未注册过，使用ADD模式，否则使用MOD模式
    int op_type = (fd_ctx->events == NONE) ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    epoll_event ep_event{};
    // 旧事件|欲添加事件 设为边缘出发
    ep_event.events = EPOLLET | fd_ctx->events | new_event;  // NOLINT
    // 自封装上下文的指针，便于在处理的地方使用该对象中的方法和属性
    ep_event.data.ptr = fd_ctx.get();
    int flag = epoll_ctl(m_epfd, op_type, filedsc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl error, m_epfd = %d, op_type = %d, filedsc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedsc, fd_ctx->events, errno);
        return -1;
    }

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
auto IOManager::DelEvent(int filedsc, EventType event) -> bool {
    m_rwlock.ReadLock();
    if (filedsc >= m_fd_contexts_vec.size()) {
        return false;
    }
    FdContext::ptr fd_ctx{m_fd_contexts_vec[filedsc]};
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
    int flag = epoll_ctl(m_epfd, op_type, filedsc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl error, m_epfd = %d, op_type = %d, filedsc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedsc, fd_ctx->events, errno);
        return false;
    }
    --m_pending_event_count;
    fd_ctx->events = new_events;
    FdContext::EventHandler& event_handler = fd_ctx->GetEventHandler(event);
    fd_ctx->ResetEventHandler(event_handler);
    return true;
}
auto IOManager::CancelEvent(int filedsc, EventType event) -> bool {
    m_rwlock.ReadLock();
    if (filedsc >= m_fd_contexts_vec.size()) {
        return false;
    }
    FdContext::ptr fd_ctx{m_fd_contexts_vec[filedsc]};
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
    ep_event.events = EPOLLIN | new_events;  // NOLINT
    ep_event.data.ptr = fd_ctx.get();
    int flag = epoll_ctl(m_epfd, op_type, filedsc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl error, m_epfd = %d, op_type = %d, filedsc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedsc, fd_ctx->events, errno);
        return false;
    }
    fd_ctx->events = new_events;
    fd_ctx->TriggerEvent(event);
    --m_pending_event_count;
    return true;
}
auto IOManager::CancleAll(int filedsc) -> bool {
    m_rwlock.ReadLock();
    if (filedsc >= m_fd_contexts_vec.size()) {
        return false;
    }
    FdContext::ptr fd_ctx{m_fd_contexts_vec[filedsc]};
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

    int flag = epoll_ctl(m_epfd, op_type, filedsc, &ep_event);
    if (flag == -1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "epoll_ctl error, m_epfd = %d, op_type = %d, filedsc "
                         "= %d, fd_ctx.events = %d, errno = %d",
                         m_epfd, op_type, filedsc, fd_ctx->events, errno);
        return false;
    }
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

auto IOManager::GetCurIOManager() -> IOManager* {
    return dynamic_cast<IOManager*>(Scheduler::GetThisThreadScheduler());
}

void IOManager::ContextVecResize(size_t size) {
    size_t old_size = m_fd_contexts_vec.size();
    WTSCLWQ_ASSERT(size = 2 * old_size, "Resize尺寸错误");
    m_fd_contexts_vec.resize(size);
    for (size_t i = old_size; i < m_fd_contexts_vec.size(); ++i) {
        m_fd_contexts_vec[i].reset(new FdContext());
        // 每个进程的fd都是从0开始分配的，其中0,1,2被占用
        m_fd_contexts_vec[i]->filedsc = static_cast<int>(i);
    }
}

void IOManager::Tickle() {}
auto IOManager::OnStop() -> bool { return false; }
void IOManager::OnIdle() {}
}  // namespace wtsclwq