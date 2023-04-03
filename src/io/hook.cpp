#pragma clang diagnostic push
#pragma ide diagnostic ignored "EmptyDeclOrStmt"

#include "../include/io/hook.h"

#include <dlfcn.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdarg>

#include "../include/config/config.h"
#include "../include/io/fd_manager.h"
#include "../include/log/log_manager.h"
#include "../include/util/macro.h"
#include "../include/util/time_util.h"

namespace wtsclwq {

static thread_local bool t_hook_enabled = false;

static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

static wtsclwq::ConfigVar<int>::ptr g_tcp_connect_timeout =
    wtsclwq::Config::Lookup("tcp.connect.timeout", 5000, "tcp 连接超时上限");

#define DEAL_FUNC(DO) /* NOLINT */ \
    DO(sleep)                      \
    DO(usleep)                     \
    DO(nanosleep)                  \
    DO(socket)                     \
    DO(connect)                    \
    DO(accept)                     \
    DO(recv)                       \
    DO(recvfrom)                   \
    DO(recvmsg)                    \
    DO(send)                       \
    DO(sendto)                     \
    DO(sendmsg)                    \
    DO(getsockopt)                 \
    DO(setsockopt)                 \
    DO(read)                       \
    DO(write)                      \
    DO(close)                      \
    DO(readv)                      \
    DO(writev)                     \
    DO(fcntl)                      \
    DO(ioctl)

void HookInit() {
    static bool is_inited = false;
    if (is_inited) {
        return;
    }

#define TRY_LOAD_HOOK_FUNC(name) /* NOLINT */ \
    name##_f = (name##_func)dlsym(RTLD_NEXT, #name);
    // 利用宏获取指定的系统 api 的函数指针
    DEAL_FUNC(TRY_LOAD_HOOK_FUNC)  // NOLINT
#undef TRY_LOAD_HOOK_FUNC
}

static uint64_t s_connect_timeout = UINT64_MAX;
struct HookIniter {
    HookIniter() {
        HookInit();
        s_connect_timeout = g_tcp_connect_timeout->GetValue();
        g_tcp_connect_timeout->AddListener(
            [](const int &old_value, const int &new_value) {
                LOG_CUSTOM_INFO(sys_logger,
                                "tcp connect timeout change from %d to %d",
                                old_value, new_value)
                s_connect_timeout = new_value;
            });
    }
};
[[maybe_unused]] static HookIniter s_hook_initer;

auto IsHookEnabled() -> bool { return t_hook_enabled; }

void SetHookEnable(bool flag) { t_hook_enabled = flag; }

}  // namespace wtsclwq

struct TimerInfo {
    int cancelled = 0;
};

template <typename OriginFunc, typename... Args>
static auto DoIO(int fd, OriginFunc func, const char *hook_func_name,  // NOLINT
                 uint32_t event, int fd_timeout_type, Args &&...args)
    -> ssize_t {
    if (!wtsclwq::IsHookEnabled()) {
        return func(fd, std::forward<Args>(args)...);
    }

    // LOG_CUSTOM_DEBUG(wtsclwq::sys_logger, "DoIO 代理执行系统函数 %s",
    // hook_func_name);

    wtsclwq::FileDescriptor::ptr fdp =
        wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd);
    if (!fdp) {
        return func(fd, std::forward<Args>(args)...);
    }
    if (fdp->IsClosed()) {
        errno = EBADF;
        return -1;
    }
    // 如果用户自己设置了非阻塞，可能是有自己的用途
    if (!fdp->IsSocket() || fdp->GetUserNonBlock()) {
        return func(fd, std::forward<Args>(args)...);
    }

    uint64_t timeout = fdp->GetTimeout(fd_timeout_type);
    // 条件的智能指针,智能指针被释放说明条件取消了
    auto timer_info = std::make_shared<TimerInfo>();
RETRY:
    int flag = func(fd, std::forward<Args>(args)...);
    // 出现错误 EINTR，是因为系统 API 在阻塞等待状态下被其他的系统信号中断执行
    // 此处的解决办法就是重新调用这次系统 API
    while (flag == -1 && errno == EINTR) {
        flag = func(fd, std::forward<Args>(args)...);
    }
    // 出现错误 EAGAIN，是因为长时间未读到数据或者无法写入数据，
    // 说明在阻塞状态,直接把这个fd 丢到 IOManager 里监听对应事件,
    // 等到事件触发后再返回当前协程上下文继续尝试
    if (flag == -1 && errno == EAGAIN) {
        LOG_CUSTOM_DEBUG(wtsclwq::sys_logger, "DoIO(%s): 开始异步等待",
                         hook_func_name);

        auto *iom = wtsclwq::IOManager::GetCurIOManager();
        wtsclwq::Timer::ptr timer;
        std::weak_ptr<TimerInfo> timer_info_wp(timer_info);
        // 如果设置了超时时间，在指定时间后取消掉该 fd 的事件监听
        if (timeout != static_cast<uint64_t>(-1)) {
            // 添加一个定时器,定时器的触发时间是该FileDescriptor的对应的event的超时时间
            // 如果这个定时器触发了,说明这次在该fd上执行的hook函数超时了
            timer = iom->AddConditionTimer(
                timeout,
                [timer_info_wp, fd, iom, event]() {
                    // 这个函数会被封装到OnTimer中,进而被调度器执行,
                    // 执行一次后,条件就会被设置为超时,然后取消并且[触发]对fd上event的监听,
                    // 确保后续的事件监听能够正确的被添加
                    // 当循环到第二次添加这个timer的时候,就不会执行if后面的语句了
                    auto time_condition = timer_info_wp.lock();
                    if (!time_condition || time_condition->cancelled != 0) {
                        return;
                    }
                    time_condition->cancelled = ETIMEDOUT;
                    iom->CancelEvent(fd,
                                     static_cast<wtsclwq::EventType>(event));
                },
                timer_info_wp);
        }
        //  如果事件触发就回到这里,因为第三个参数不设置的话,事件的回调默认是回到cur_fiber
        int ret = iom->AddEvent(fd, static_cast<wtsclwq::EventType>(event));
        if (ret == -1) {
            LOG_CUSTOM_ERROR(wtsclwq::sys_logger, "%s addEventListener(%d, %u)",
                             hook_func_name, fd, event);
            if (timer) {
                timer->Cancel();
            }
            return -1;
        }

        // 有两种情况可以回到这里:
        // 1.定时器任务执行完,在定时器中执行事件的Cancel,然后强制事件回调触发==>超时
        // 2.iomanager.idle中触发, 正常完成事件回调触发==>正常
        wtsclwq::Fiber::YieldToHoldBackScheduler();
        if (timer) {
            timer->Cancel();
        }
        // 上面的情况2,发生超时,直接返回-1
        if (timer_info->cancelled != 0) {
            errno = timer_info->cancelled;
            return -1;
        }
        goto RETRY;  // NOLINT
    }
    LOG_CUSTOM_DEBUG(wtsclwq::sys_logger, "DoIO end errno %d", errno);
    return flag;
}

extern "C" {
#define DEF_FUNC_NAME(name) name##_func name##_f = nullptr;  // NOLINT
// 定义系统 api 的函数指针的变量
DEAL_FUNC(DEF_FUNC_NAME)
#undef DEF_FUNC_NAME

/**
 * @brief hook 处理后的 sleep，利用 IOManager
 * 的定时器功能实现，创建一个延迟时间为 seconds
 * 的定时器，用于将本协程重新加入调度，随后便换出当前协程
 */
auto sleep(unsigned int seconds) -> unsigned int {
    if (!wtsclwq::IsHookEnabled()) {
        return sleep_f(seconds);
    }
    wtsclwq::Fiber::ptr fiber = wtsclwq::Fiber::GetCurFiber();
    auto *iom = wtsclwq::IOManager::GetCurIOManager();
    WTSCLWQ_ASSERT(iom != nullptr, "这里的 IOManager 指针不可为空");
    iom->AddTimer(static_cast<uint64_t>(seconds) * BASE_NUMBER_OF_SECONDS,
                  [iom, fiber]() { iom->Schedule(fiber); });
    wtsclwq::Fiber::YieldToHoldBackScheduler();
    return 0;
}

/**
 * @brief hook 处理后的 usleep
 */
auto usleep(useconds_t useconds) -> int {
    if (!wtsclwq::IsHookEnabled()) {
        return usleep_f(useconds);
    }
    wtsclwq::Fiber::ptr fiber = wtsclwq::Fiber::GetCurFiber();
    auto *iom = wtsclwq::IOManager::GetCurIOManager();
    WTSCLWQ_ASSERT(iom != nullptr, "这里的 IOManager 指针不可为空");
    iom->AddTimer(static_cast<uint64_t>(useconds) / BASE_NUMBER_OF_SECONDS,
                  [iom, fiber]() { iom->Schedule(fiber); });
    wtsclwq::Fiber::YieldToHoldBackScheduler();
    return 0;
}

auto nanosleep(const struct timespec *requested_time,
               struct timespec *remaining) -> int {
    if (!wtsclwq::IsHookEnabled()) {
        return nanosleep_f(requested_time, remaining);
    }
    uint64_t timeout_ms = requested_time->tv_sec * BASE_NUMBER_OF_SECONDS +
                          requested_time->tv_nsec / BASE_NUMBER_OF_SECONDS /
                              BASE_NUMBER_OF_SECONDS;
    wtsclwq::Fiber::ptr fiber = wtsclwq::Fiber::GetCurFiber();
    auto *iom = wtsclwq::IOManager::GetCurIOManager();
    WTSCLWQ_ASSERT(iom != nullptr, "这里的 IOManager 指针不可为空");
    iom->AddTimer(timeout_ms, [iom, fiber]() { iom->Schedule(fiber); });
    wtsclwq::Fiber::YieldToHoldBackScheduler();
    return 0;
}

//////// sys/socket.h
auto socket(int domain, int type, int protocol) -> int {
    if (!wtsclwq::IsHookEnabled()) {
        return socket_f(domain, type, protocol);
    }
    int filedesc = socket_f(domain, type, protocol);
    if (filedesc == -1) {
        return filedesc;
    }
    wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(filedesc, true);
    return filedesc;
}

auto ConnectWithTimeout(int sockfd, const struct sockaddr *addr,  // NOLINT
                        socklen_t addrlen, uint64_t timeout_ms) -> int {
    if (!wtsclwq::IsHookEnabled()) {
        LOG_INFO(wtsclwq::sys_logger, "t_hook_enabled is false");
        return connect_f(sockfd, addr, addrlen);
    }
    auto fdp = wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(sockfd);
    if (!fdp || fdp->IsClosed()) {
        errno = EBADF;
        return -1;
    }
    if (!fdp->IsSocket()) {
        LOG_CUSTOM_INFO(wtsclwq::sys_logger, "fd :%d not socket", sockfd);
        return connect_f(sockfd, addr, addrlen);
    }
    if (fdp->GetUserNonBlock()) {
        LOG_CUSTOM_INFO(wtsclwq::sys_logger, "fd :%d is user non-blocked",
                        sockfd);
        return connect_f(sockfd, addr, addrlen);
    }
    int flag = connect_f(sockfd, addr, addrlen);
    if (flag == 0) {
        return 0;
    }
    if (flag != -1 || errno != EINPROGRESS) {
        return flag;
    }

    /**
     * 调用 connect，非阻塞形式下会返回-1，但是 errno 被设为 EINPROGRESS，表明
     * connect 仍旧在进行还没有完成。 下一步就需要为其添加 write
     * 事件监听，当连接成功后会触发该事件。
     */
    auto *iom = wtsclwq::IOManager::GetCurIOManager();
    wtsclwq::Timer::ptr timer;
    auto timer_info = std::make_shared<TimerInfo>();
    std::weak_ptr<TimerInfo> weak_timer_info(timer_info);

    if (timeout_ms != static_cast<uint64_t>(-1)) {
        timer = iom->AddConditionTimer(
            timeout_ms,
            [weak_timer_info, sockfd, iom]() {
                auto time_condition = weak_timer_info.lock();
                if (!time_condition || (time_condition->cancelled != 0)) {
                    return;
                }
                time_condition->cancelled = ETIMEDOUT;
                iom->CancelEvent(sockfd, wtsclwq::EventType::WRITE);
            },
            weak_timer_info);
    }

    int ret = iom->AddEvent(sockfd, wtsclwq::EventType::WRITE);
    if (ret == 0) {
        wtsclwq::Fiber::YieldToHoldBackScheduler();
        if (timer) {
            timer->Cancel();
        }
        if (timer_info->cancelled != 0) {
            errno = timer_info->cancelled;
            return -1;
        }
    }
    if (ret == -1) {
        if (timer) {
            timer->Cancel();
        }
        LOG_CUSTOM_ERROR(wtsclwq::sys_logger,
                         "connectWithTimeout addEventListener(%d, write) error",
                         sockfd);
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == -1) {
        return -1;
    }

    if (error == 0) {
        return 0;
    }
    errno = error;
    return -1;
}

auto connect(int fd, const struct sockaddr *addr, socklen_t len) -> int {
    return ConnectWithTimeout(fd, addr, len, wtsclwq::s_connect_timeout);
}

auto accept(int fd, struct sockaddr *addr, socklen_t *len) -> int {
    ssize_t flag = DoIO(fd, accept_f, "accept", wtsclwq::EventType::READ,
                        SO_RCVTIMEO, addr, len);
    if (flag >= 0) {
        wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd, true);
    }
    return static_cast<int>(flag);
}

auto read(int fd, void *buf, size_t nbytes) -> ssize_t {
    return DoIO(fd, read_f, "read", wtsclwq::EventType::READ, SO_RCVTIMEO, buf,
                nbytes);
}

auto readv(int fd, const struct iovec *iovec, int count) -> ssize_t {
    return DoIO(fd, readv_f, "readv", wtsclwq::EventType::READ, SO_RCVTIMEO,
                iovec, count);
}

auto recv(int fd, void *buf, size_t n, int flags) -> ssize_t {
    return DoIO(fd, recv_f, "recv", wtsclwq::EventType::READ, SO_RCVTIMEO, buf,
                n, flags);
}

auto recvfrom(int fd, void *buf, size_t n, int flags, struct sockaddr *addr,
              socklen_t *addr_len) -> ssize_t {
    return DoIO(fd, recvfrom_f, "recvfrom", wtsclwq::EventType::READ,
                SO_RCVTIMEO, buf, n, flags, addr, addr_len);
}

auto recvmsg(int fd, struct msghdr *message, int flags) -> ssize_t {
    return DoIO(fd, recvmsg_f, "recvfrom", wtsclwq::EventType::READ,
                SO_RCVTIMEO, message, flags);
}

auto write(int fd, const void *buf, size_t n) -> ssize_t {
    return DoIO(fd, write_f, "write", wtsclwq::EventType::WRITE, SO_SNDTIMEO,
                buf, n);
}

auto writev(int fd, const struct iovec *iovec, int count) -> ssize_t {
    return DoIO(fd, writev_f, "writev_f", wtsclwq::EventType::WRITE,
                SO_SNDTIMEO, iovec, count);
}

auto send(int fd, const void *buf, size_t n, int flags) -> ssize_t {
    return DoIO(fd, send_f, "send", wtsclwq::EventType::WRITE, SO_SNDTIMEO, buf,
                n, flags);
}

auto sendto(int fd, const void *buf, size_t n, int flags,
            const struct sockaddr *addr, socklen_t addr_len) -> ssize_t {
    return DoIO(fd, sendto_f, "sendto", wtsclwq::EventType::WRITE, SO_SNDTIMEO,
                buf, n, flags, addr, addr_len);
}

auto sendmsg(int fd, const struct msghdr *message, int flags) -> ssize_t {
    return DoIO(fd, sendmsg_f, "sendmsg", wtsclwq::EventType::WRITE,
                SO_SNDTIMEO, message, flags);
}

auto close(int fd) -> int {
    if (!wtsclwq::IsHookEnabled()) {
        return close_f(fd);
    }
    wtsclwq::FileDescriptor::ptr fdp =
        wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd);
    if (fdp) {
        auto *iom = wtsclwq::IOManager::GetCurIOManager();
        if (iom != nullptr) {
            iom->CancelAll(fd);
        }
        wtsclwq::FileDescriptorManager::GetInstancePtr()->Remove(fd);
    }
    return close_f(fd);
}

auto fcntl(int fd, int cmd, ... /* arg */) -> int {
    va_list va;
    va_start(va, cmd);
    switch (cmd) {
        case F_SETFL: {
            int arg = va_arg(va, int);
            va_end(va);
            auto fdp =
                wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd);
            if (!fdp || fdp->IsClosed() || !fdp->IsSocket()) {
                return fcntl_f(fd, cmd, arg);
            }
            fdp->SetUserNonBlock((arg & O_NONBLOCK) != 0);
            // 根据这个 fd 的包装对象是否设置了系统层面的非阻塞，
            // socket fd 会被强制设置为非阻塞，其他则不一定
            if (fdp->GetSystemNonBlock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETFL: {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            auto fdp =
                wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd);
            if (!fdp || fdp->IsClosed() || !fdp->IsSocket()) {
                return arg;
            }
            if (fdp->GetUserNonBlock()) {
                return arg | O_NONBLOCK;
            }
            return arg & ~O_NONBLOCK;
        }
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
        case F_SETPIPE_SZ: {
            int arg = va_arg(va, int);  // NOLINT
            va_end(va);                 // NOLINT
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
        case F_GETPIPE_SZ: {
            va_end(va);  // NOLINT
            return fcntl_f(fd, cmd);
        }
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            auto *arg = va_arg(va, flock *);  // NOLINT
            va_end(va);                       // NOLINT
            return fcntl_f(fd, cmd, arg);
        }
        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            auto *arg = va_arg(va, f_owner_ex *);  // NOLINT
            va_end(va);                            // NOLINT
            return fcntl_f(fd, cmd, arg);
        }
        default:
            va_end(va);  // NOLINT
            return fcntl_f(fd, cmd);
    }
}

auto ioctl(int fd, unsigned long request, ...) -> int {  // NOLINT
    va_list va;
    va_start(va, request);                               // NOLINT
    void *arg = va_arg(va, void *);                      // NOLINT
    va_end(va);                                          // NOLINT

    if (FIONBIO == request) {
        bool user_nonblock = *static_cast<int *>(arg) != 0;
        auto fdp = wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd);
        if (!fdp || fdp->IsClosed() || !fdp->IsSocket()) {
            return ioctl_f(fd, request, arg);
        }
        fdp->SetUserNonBlock(user_nonblock);
    }
    return ioctl(fd, request, arg);
}

auto getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
    -> int {
    return getsockopt_f(fd, level, optname, optval, optlen);
}

auto setsockopt(int fd, int level, int optname, const void *optval,
                socklen_t optlen) -> int {
    if (!wtsclwq::IsHookEnabled()) {
        return setsockopt_f(fd, level, optname, optval, optlen);
    }
    //
    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            auto fdp =
                wtsclwq::FileDescriptorManager::GetInstancePtr()->Get(fd);
            if (fdp) {
                const auto *v = static_cast<const timeval *>(optval);
                fdp->SetTimeout(optname,
                                v->tv_sec * BASE_NUMBER_OF_SECONDS +
                                    v->tv_usec / BASE_NUMBER_OF_SECONDS);
            }
        }
    }
    return setsockopt_f(fd, level, optname, optval, optlen);
}

}  // extern "C"

#pragma clang diagnostic pop