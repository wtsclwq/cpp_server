//
// Created by wtsclwq on 23-3-29.
//

#include "../include/socket/socket.h"

#include <netinet/tcp.h>
#include <sys/socket.h>

#include "../include/io/fd_manager.h"
#include "../include/io/hook.h"
#include "../include/log/log_manager.h"
#include "../include/socket/ipv4_address.h"
#include "../include/socket/ipv6_address.h"
#include "../include/socket/unix_address.h"
#include "../include/socket/unknow_address.h"
#include "../include/util/fs_util.h"
#include "../include/util/time_util.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

Socket::Socket(int family, int type, int protocol)
    : m_family(family), m_type(type), m_protocol(protocol) {}

Socket::~Socket() {
    if (m_socket != -1) {
        ::close(m_socket);
    }
}

auto Socket::CreateTcpSocket(const Address::ptr &address) -> Socket::ptr {
    return std::make_shared<Socket>(address->GetFamily(), TCP, 0);
}

auto Socket::CreateUdpSocket(const Address::ptr &address) -> Socket::ptr {
    Socket::ptr socket = std::make_shared<Socket>(address->GetFamily(), UDP, 0);
    socket->NewSocket();
    socket->m_is_connected = true;
    return socket;
}

auto Socket::CreateIpv4TcpSocket() -> Socket::ptr {
    return std::make_shared<Socket>(IPV4, TCP, 0);
}

auto Socket::CreateIpv4UdpSocket() -> Socket::ptr {
    return std::make_shared<Socket>(IPV4, UDP, 0);
}

auto Socket::CreateIpv6TcpSocket() -> Socket::ptr {
    return std::make_shared<Socket>(IPV6, TCP, 0);
}

auto Socket::CreateIpv6UdpSocket() -> Socket::ptr {
    return std::make_shared<Socket>(IPV6, UDP, 0);
}

auto Socket::CreateUnixTcpSocket() -> Socket::ptr {
    return std::make_shared<Socket>(UNIX, TCP, 0);
}

auto Socket::CreateUnixUdpSocket() -> Socket::ptr {
    return std::make_shared<Socket>(UNIX, UDP, 0);
}

auto Socket::GetSendTimeout() const -> uint64_t {
    FileDescriptor::ptr fdp =
        FileDescriptorManager ::GetInstancePtr()->Get(m_socket);
    if (fdp != nullptr) {
        // 获取的时候其实使用fd_manager或者getsockopt是一样的，因为一定是先设置，设置的时候就同时更改了内核和fd_manager
        return fdp->GetTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::SetSendTimeout(uint64_t timeout) {
    struct timeval tv {
        static_cast<int>(timeout / BASE_NUMBER_OF_SECONDS),
            static_cast<int>(timeout % BASE_NUMBER_OF_SECONDS *
                             BASE_NUMBER_OF_SECONDS)
    };
    // 注意这里设置的时候直接调用setsockopt是因为我们hook了这个函数，里面会对fd_manager进行操作
    SetOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

auto Socket::GetRecvTimeout() const -> uint64_t {
    FileDescriptor::ptr fdp =
        FileDescriptorManager ::GetInstancePtr()->Get(m_socket);
    if (fdp != nullptr) {
        // 获取的时候其实使用fd_manager或者getsockopt是一样的，因为一定是先设置，设置的时候就同时更改了内核和fd_manager
        return fdp->GetTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::SetRecvTimeout(uint64_t timeout) {
    struct timeval tv {
        static_cast<int>(timeout / BASE_NUMBER_OF_SECONDS),
            static_cast<int>(timeout % BASE_NUMBER_OF_SECONDS *
                             BASE_NUMBER_OF_SECONDS)
    };
    // 注意这里设置的时候直接调用setsockopt是因为我们hook了这个函数，里面会对fd_manager进行操作
    SetOption(SOL_SOCKET, SO_RCVTIMEO, tv);
}

auto Socket::GetOptionWithLen(int level, int option, void *result,
                              socklen_t *len) const -> bool {
    int flag = getsockopt(m_socket, level, option, result, len);
    if (flag != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "GetOption socket = %d,"
                         "level = %d, option = %d, errno = %d, errstr = %s",
                         m_socket, level, option, errno, strerror(errno))
        return false;
    }
    return true;
}

auto Socket::SetOptionWithLen(int level, int option, const void *result,
                              socklen_t socklen) -> bool {
    int flag = setsockopt(m_socket, level, option, result, socklen);
    if (flag != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "SetOption socket = %d,"
                         "level = %d, option = %d, errno = %d, errstr = %s",
                         m_socket, level, option, errno, strerror(errno))
        return false;
    }
    return true;
}

auto Socket::Bind(const Address::ptr &address) -> bool {
    if (!IsValid()) {
        NewSocket();
    }
    if (!IsValid()) {
        return false;
    }

    if (address->GetFamily() != m_family) {
        LOG_CUSTOM_ERROR(
            sys_logger,
            "Bind m_socket.family(%d), addr.family(%d), not equal, addr = %s",
            m_family, address->GetFamily(), address->ToString().c_str())
        return false;
    }

    auto unix_address = std::dynamic_pointer_cast<UnixAddress>(address);
    // 如果地址是一个unix地址
    if (unix_address != nullptr) {
        // TODO 没太看懂
        Socket::ptr socket = Socket::CreateUnixTcpSocket();
        if (socket->Connect(unix_address, UINT64_MAX)) {
            return false;
        }
        FsUtil::Unlink(unix_address->GetPath(), true);
    }

    if (::bind(m_socket, address->GetAddr(), address->GetAddrLen()) != 0) {
        LOG_CUSTOM_ERROR(sys_logger, "bind error: errno = %d, errstr = %s",
                         errno, strerror(errno))
        return false;
    }
    GetLocalAddress();
    return true;
}

auto Socket::Listen(int backlog) -> bool {
    if (!IsValid()) {
        LOG_ERROR(sys_logger, "listen error sock = -1");
        return false;
    }
    if (::listen(m_socket, backlog) != 0) {
        LOG_CUSTOM_ERROR(sys_logger, "listen errno errno = %d, errstr = %s",
                         errno, strerror(errno))
        return false;
    }
    return true;
}

auto Socket::Accept() -> Socket::ptr {
    Socket::ptr new_socket =
        std::make_shared<Socket>(m_family, m_type, m_protocol);
    int new_raw_socket = ::accept(m_socket, nullptr, nullptr);
    if (new_raw_socket == -1) {
        LOG_CUSTOM_ERROR(sys_logger, "accept(%d) errno = %d, errstr = %s",
                         new_raw_socket, errno, strerror(errno))
        return nullptr;
    }
    // 初始化这个新的socket
    if (new_socket->Init(new_raw_socket)) {
        return new_socket;
    }
    return nullptr;
}

auto Socket::Connect(const Address::ptr &address, uint64_t timeout) -> bool {
    // 远端地址就是要connect的地址
    m_remote_address = address;
    if (!IsValid()) {
        NewSocket();
    }
    if (!IsValid()) {
        return false;
    }
    if (address->GetFamily() != m_family) {
        LOG_CUSTOM_ERROR(
            sys_logger,
            "connect m_family = %d, address.family = %d not equal, addr = %s",
            m_family, address->GetFamily(), address->ToString().c_str())
        return false;
    }

    if (timeout == UINT64_MAX) {
        if (::connect(m_socket, address->GetAddr(), address->GetAddrLen()) !=
            0) {
            LOG_CUSTOM_ERROR(
                sys_logger,
                "sock = %d, connect(%s), error, errno = %d, errstr = %s",
                m_socket, address->ToString().c_str(), errno, strerror(errno))
            Close();
            return false;
        }
    } else {
        if (::ConnectWithTimeout(m_socket, address->GetAddr(),
                                 address->GetAddrLen(), timeout) != 0) {
            LOG_CUSTOM_ERROR(sys_logger,
                             "sock = %d, ConnectWithTimeout(%s,%lu), error, "
                             "errno = %d, errstr = %s",
                             m_socket, address->ToString().c_str(), timeout,
                             errno, strerror(errno))
            Close();
            return false;
        }
    }
    m_is_connected = true;
    GetRemoteAddress();
    GetLocalAddress();
    return true;
}

auto Socket::Reconnect(uint64_t timeout) -> bool {
    if (m_remote_address == nullptr) {
        LOG_ERROR(sys_logger, "reconnect m_remote_address is nullptr");
        return false;
    }
    m_local_address.reset();
    return Connect(m_remote_address, timeout);
}

auto Socket::Close() -> bool {
    if (!m_is_connected && m_socket == -1) {
        return true;
    }
    m_is_connected = false;
    if (m_socket != -1) {
        int flag = ::close(m_socket);
        if (flag != 0) {
            return false;
        }
        m_socket = -1;
    }
    return true;
}

auto Socket::Send(const void *buffer, size_t length, int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    return ::send(m_socket, buffer, length, flags);
}

auto Socket::SendIovec(iovec *buffers, size_t length, int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    msghdr msg{};
    msg.msg_iov = buffers;
    msg.msg_iovlen = length;
    return ::sendmsg(m_socket, &msg, flags);
}

auto Socket::SendTo(const void *buffer, size_t length, const Address::ptr &to,
                    int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    return ::sendto(m_socket, buffer, length, flags, to->GetAddr(),
                    to->GetAddrLen());
}

auto Socket::SendIovecTo(iovec *buffers, size_t length, const Address::ptr &to,
                         int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    msghdr msg{};
    msg.msg_iov = buffers;
    msg.msg_iovlen = length;
    msg.msg_name = to->GetAddr();
    msg.msg_namelen = to->GetAddrLen();
    return ::sendmsg(m_socket, &msg, flags);
}

auto Socket::Recv(void *buffer, size_t length, int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    return ::recv(m_socket, buffer, length, flags);
}

auto Socket::RecvIovec(iovec *buffers, size_t length, int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    msghdr msg{};
    msg.msg_iov = buffers;
    msg.msg_iovlen = length;
    return recvmsg(m_socket, &msg, flags);
}

auto Socket::RecvFrom(void *buffer, size_t length, const Address::ptr &from,
                      int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    socklen_t len = from->GetAddrLen();
    return ::recvfrom(m_socket, buffer, length, flags, from->GetAddr(), &len);
}

auto Socket::RecvIovecFrom(iovec *buffers, size_t length,
                           const Address::ptr &from, int flags) -> ssize_t {
    if (!IsConnected()) {
        return -1;
    }
    msghdr msg{};
    msg.msg_iov = buffers;
    msg.msg_iovlen = length;
    msg.msg_name = from->GetAddr();
    msg.msg_namelen = from->GetAddrLen();
    return ::recvmsg(m_socket, &msg, flags);
}

auto Socket::Dump(std::ostream &os) const -> std::ostream & {
    os << "[Socket sock=" << m_socket << " is_connected=" << m_is_connected
       << " family=" << m_family << " type=" << m_type
       << " protocol=" << m_protocol;
    if (m_local_address) {
        os << " local_address=" << m_local_address->ToString();
    }
    if (m_remote_address) {
        os << " remote_address=" << m_remote_address->ToString();
    }
    os << "]";
    return os;
}

auto Socket::ToString() const -> std::string {
    std::stringstream ss;
    Dump(ss);
    return ss.str();
}

auto Socket::GetRemoteAddress() -> Address::ptr {
    if (m_remote_address != nullptr) {
        return m_remote_address;
    }
    Address::ptr result;
    switch (m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnKnowAddress());
            break;
    }
    socklen_t addrlen = result->GetAddrLen();
    //  如果在指定socket上无法获取到远端地址
    // getpeername()函数用于获取与一个已连接套接字（connected
    // socket）相关联的远程主机（remote host）信息
    if (getpeername(m_socket, result->GetAddr(), &addrlen) != 0) {
        return std::make_shared<UnKnowAddress>(m_family);
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->SetAddrLen(addrlen);
    }
    m_remote_address = result;
    return m_remote_address;
}

auto Socket::GetLocalAddress() -> Address::ptr {
    if (m_local_address != nullptr) {
        return m_local_address;
    }
    Address::ptr result;
    switch (m_family) {
        case AF_INET:
            result.reset(new IPv4Address());
            break;
        case AF_INET6:
            result.reset(new IPv6Address());
            break;
        case AF_UNIX:
            result.reset(new UnixAddress());
            break;
        default:
            result.reset(new UnKnowAddress());
            break;
    }
    socklen_t addrlen = result->GetAddrLen();
    //  如果在指定socket上无法获取到远端地址
    // getsockname()函数用于获取一个已绑定套接字（bound socket）的本地地址信息
    if (getsockname(m_socket, result->GetAddr(), &addrlen) != 0) {
        return std::make_shared<UnKnowAddress>(m_family);
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->SetAddrLen(addrlen);
    }
    m_local_address = result;
    return m_local_address;
}

auto Socket::GetSocket() const -> int { return m_socket; }

auto Socket::GetFamily() const -> int { return m_family; }

auto Socket::GetType() const -> int { return m_type; }

auto Socket::GetProtocol() const -> int { return m_protocol; }

auto Socket::IsConnected() const -> bool { return m_is_connected; }

auto Socket::IsValid() const -> bool { return m_socket != -1; }

auto Socket::GetError() const -> int {
    int error = 0;
    socklen_t len = sizeof(error);
    if (GetOptionWithLen(SOL_SOCKET, SO_ERROR, &error, &len)) {
        error = errno;
    }
    return error;
}

auto Socket::CancelRead() -> bool {
    return IOManager::GetCurIOManager()->CancelEvent(m_socket, wtsclwq::READ);
}

auto Socket::CancelWrite() -> bool {
    return IOManager::GetCurIOManager()->CancelEvent(m_socket, wtsclwq::WRITE);
}

auto Socket::CancelAccept() -> bool {
    return IOManager::GetCurIOManager()->CancelEvent(m_socket, wtsclwq::READ);
}

auto Socket::CancelAll() -> bool {
    return IOManager::GetCurIOManager()->CancleAll(m_socket);
}

void Socket::InitSocket() {
    int val = 1;
    SetOption(SOL_SOCKET, SO_REUSEADDR, val);
    if (m_type == SOCK_STREAM) {
        // 禁用 Nagle 算法，即开启了数据无延迟发送模式。
        SetOption(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::NewSocket() {
    m_socket = ::socket(m_family, m_type, m_protocol);
    if (m_socket != -1) {
        InitSocket();
    } else {
        LOG_CUSTOM_ERROR(sys_logger, "socket(%d,%d,%d) errno = %d, errstr = %s",
                         m_family, m_type, m_protocol, errno, strerror(errno))
    }
}

auto Socket::Init(int sock) -> bool {
    FileDescriptor::ptr fdp =
        FileDescriptorManager ::GetInstancePtr()->Get(sock);
    if (fdp == nullptr || fdp->IsSocket() || !fdp->IsClosed()) {
        return false;
    }
    m_socket = sock;
    m_is_connected = true;
    InitSocket();
    GetLocalAddress();
    GetRemoteAddress();
    return true;
}
}  // namespace wtsclwq