//
// Created by wtsclwq on 23-3-31.
//

#include "../include/socket/ssl_socket.h"
namespace wtsclwq {
auto SSLSocket::CreateTcpSslSocket(Address::ptr address) -> SSLSocket::ptr {
    return {};
}
auto SSLSocket::CreateIpv4TcpSslSocket() -> SSLSocket::ptr {
    return {};
}
auto SSLSocket::CreateIpv6TcpSslSocket() -> SSLSocket::ptr {
    return {};
}
SSLSocket::SSLSocket(int family, int type, int protocol)
    : Socket(family, type, protocol) {}

auto SSLSocket::Bind(const Address::ptr &address) -> bool {
    return Socket::Bind(address);
}

auto SSLSocket::Listen(int backlog) -> bool { return Socket::Listen(backlog); }

auto SSLSocket::Accept() -> Socket::ptr { return Socket::Accept(); }

auto SSLSocket::Connect(const Address::ptr &address, uint64_t timeout) -> bool {
    return Socket::Connect(address, timeout);
}

auto SSLSocket::Reconnect(uint64_t timeout) -> bool {
    return Socket::Reconnect(timeout);
}

auto SSLSocket::Close() -> bool { return Socket::Close(); }

auto SSLSocket::Send(const void *buffer, size_t length, int flags) -> ssize_t {
    return Socket::Send(buffer, length, flags);
}

auto SSLSocket::SendIovec(iovec *buffers, size_t length, int flags)
    -> ssize_t {
    return Socket::SendIovec(buffers, length, flags);
}

auto SSLSocket::SendTo(const void *buffer, size_t length,
                       const Address::ptr &to, int flags) -> ssize_t {
    return Socket::SendTo(buffer, length, to, flags);
}

auto SSLSocket::SendIovecTo(iovec *buffers, size_t length,
                            const Address::ptr &to, int flags) -> ssize_t {
    return Socket::SendIovecTo(buffers, length, to, flags);
}

auto SSLSocket::Recv(void *buffer, size_t length, int flags) -> ssize_t {
    return Socket::Recv(buffer, length, flags);
}

auto SSLSocket::RecvIovec(iovec *buffers, size_t length, int flags) -> ssize_t {
    return Socket::RecvIovec(buffers, length, flags);
}

auto SSLSocket::RecvFrom(void *buffer, size_t length, const Address::ptr &from,
                         int flags) -> ssize_t {
    return Socket::RecvFrom(buffer, length, from, flags);
}

auto SSLSocket::RecvIovecFrom(iovec *buffers, size_t length,
                              const Address::ptr &from, int flags) -> ssize_t {
    return Socket::RecvIovecFrom(buffers, length, from, flags);
}

auto SSLSocket::Dump(std::ostream &os) const -> std::ostream & {
    return Socket::Dump(os);
}

auto SSLSocket::ToString() const -> std::string { return Socket::ToString(); }

auto SSLSocket::Init(int sock) -> bool { return Socket::Init(sock); }
}  // namespace wtsclwq