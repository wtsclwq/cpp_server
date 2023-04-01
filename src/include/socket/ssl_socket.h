//
// Created by wtsclwq on 23-3-31.
//

#pragma once

#include <openssl/types.h>

#include <memory>

#include "socket.h"
namespace wtsclwq {

class SslSocket : public Socket {
  public:
    using ptr = std::shared_ptr<SslSocket>;
    SslSocket(const SslSocket &other) = delete;
    SslSocket(SslSocket *&other) = delete;
    auto operator=(const SslSocket &other) = delete;
    auto operator=(SslSocket &&other) = delete;
    SslSocket() = default;
    ~SslSocket() override = default;

    SslSocket(int family, int type, int protocol = 0);
    /**
     * @brief 创建一个TcpSslSocket
     */
    static auto CreateTcpSslSocket(const Address::ptr &address)
        -> SslSocket::ptr;

    /**
     * @brief 创建一个Ipv4的TcpSslSocket
     */
    static auto CreateIpv4TcpSslSocket() -> SslSocket::ptr;

    /**
     * @brief 创建一个Ipv6的TcpSslSocket
     */
    static auto CreateIpv6TcpSslSocket() -> SslSocket::ptr;

    auto Bind(const Address::ptr &address) -> bool override;

    auto Listen(int backlog) -> bool override;

    auto Accept() -> Socket::ptr override;

    auto Connect(const Address::ptr &address, uint64_t timeout)
        -> bool override;

    auto Reconnect(uint64_t timeout) -> bool override;

    auto Close() -> bool override;

    auto Send(const void *buffer, size_t length, int flags) -> ssize_t override;

    auto SendIovec(iovec *buffers, size_t length, int flags)
        -> ssize_t override;

    auto SendTo(const void *buffer, size_t length, const Address::ptr &to,
                int flags) -> ssize_t override;

    auto SendIovecTo(iovec *buffers, size_t length, const Address::ptr &to,
                     int flags) -> ssize_t override;

    auto Recv(void *buffer, size_t length, int flags) -> ssize_t override;

    auto RecvIovec(iovec *buffers, size_t length, int flags)
        -> ssize_t override;

    auto RecvFrom(void *buffer, size_t length, const Address::ptr &from,
                  int flags) -> ssize_t override;

    auto RecvIovecFrom(iovec *buffers, size_t length, const Address::ptr &from,
                       int flags) -> ssize_t override;

    auto Dump(std::ostream &os) const -> std::ostream & override;

    auto ToString() const -> std::string override;

    auto LoadCertificates(const std::string &cert_file,
                          const std::string &key_file) -> bool;

  protected:
    auto Init(int sock) -> bool override;

  private:
    std::shared_ptr<SSL_CTX> m_ctx;  // SSL上下文
    std::shared_ptr<SSL> m_ssl;      // SSL
};
}  // namespace wtsclwq