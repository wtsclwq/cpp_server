#ifndef SERVER_FRAMEWORK_HOOK_H
#define SERVER_FRAMEWORK_HOOK_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstdint>
#include <ctime>

namespace wtsclwq {

/**
 * @brief 检查当前线程是否开启系统函数 hook 到协程的实现
 */
auto IsHookEnabled() -> bool;

/**
 * @brief 开启系统函数 hook 到协程的实现
 */
void SetHookEnable(bool flag);

}  // namespace wtsclwq

extern "C" {
//////// time.h
using sleep_func = unsigned int (*)(unsigned int);
extern sleep_func sleep_f;

using usleep_func = int (*)(useconds_t);
extern usleep_func usleep_f;

using nanosleep_func = int (*)(const struct timespec *, struct timespec *);
extern nanosleep_func nanosleep_f;

//////// sys/socket.h
using socket_func = int (*)(int, int, int);
extern socket_func socket_f;

using connect_func = int (*)(int, const struct sockaddr *, socklen_t);
extern connect_func connect_f;

using accept_func = int (*)(int, struct sockaddr *, socklen_t *);
extern accept_func accept_f;

using recv_func = ssize_t (*)(int, void *, size_t, int);
extern recv_func recv_f;

using recvfrom_func = ssize_t (*)(int, void *, size_t, int, struct sockaddr *,
                                  socklen_t *);
extern recvfrom_func recvfrom_f;

using recvmsg_func = ssize_t (*)(int, struct msghdr *, int);
extern recvmsg_func recvmsg_f;

using send_func = ssize_t (*)(int, const void *, size_t, int);
extern send_func send_f;

using sendto_func = ssize_t (*)(int, const void *, size_t, int,
                                const struct sockaddr *, socklen_t);
extern sendto_func sendto_f;

using sendmsg_func = ssize_t (*)(int, const struct msghdr *, int);
extern sendmsg_func sendmsg_f;

using getsockopt_func = int (*)(int, int, int, void *, socklen_t *);
extern getsockopt_func getsockopt_f;

using setsockopt_func = int (*)(int, int, int, const void *, socklen_t);
extern setsockopt_func setsockopt_f;

extern auto ConnectWithTimeout(int fd, const struct sockaddr *addr,
                               socklen_t addrlen, uint64_t timeout_ms) -> int;

//////// unistd.h
using read_func = ssize_t (*)(int, void *, size_t);
extern read_func read_f;

using write_func = ssize_t (*)(int, const void *, size_t);
extern write_func write_f;

using close_func = int (*)(int);
extern close_func close_f;

//////// sys/uio.h
using readv_func = ssize_t (*)(int, const struct iovec *, int);
extern readv_func readv_f;

using writev_func = ssize_t (*)(int, const struct iovec *, int);
extern writev_func writev_f;

//////// fcntl.h
using fcntl_func = int (*)(int, int, ...);
extern fcntl_func fcntl_f;

//////// sys/ioctl.h
using ioctl_func = int (*)(int, uint64_t, ...);
extern ioctl_func ioctl_f;
}

#endif
