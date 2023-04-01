//
// Created by wtsclwq on 23-3-31.
//

#include "../include/socket/ssl_socket.h"

#include "../include/log/log_manager.h"
#include "openssl/ssl.h"

namespace wtsclwq {
Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

struct SslIniter {
    SslIniter() {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
    }
};

static SslIniter s_ssl_inter{};

SslSocket::SslSocket(int family, int type, int protocol)
    : Socket(family, type, protocol) {}

auto SslSocket::CreateTcpSslSocket(const Address::ptr &address)
    -> SslSocket::ptr {
    return std::make_shared<SslSocket>(address->GetFamily(), TCP, 0);
}

auto SslSocket::CreateIpv4TcpSslSocket() -> SslSocket::ptr {
    return std::make_shared<SslSocket>(IPV4, TCP, 0);
}

auto SslSocket::CreateIpv6TcpSslSocket() -> SslSocket::ptr {
    return std::make_shared<SslSocket>(IPV6, TCP, 0);
}

auto SslSocket::Bind(const Address::ptr &address) -> bool {
    return Socket::Bind(address);
}

auto SslSocket::Listen(int backlog) -> bool { return Socket::Listen(backlog); }

auto SslSocket::Accept() -> Socket::ptr {
    SslSocket::ptr new_socket =
        std::make_shared<SslSocket>(m_family, m_type, m_protocol);
    int new_raw_socket = ::accept(m_socket, nullptr, nullptr);
    if (new_raw_socket == -1) {
        LOG_CUSTOM_ERROR(
            sys_logger, "SslSocket::Accept accept(%d), errno = %d, errstr = %s",
            m_socket, errno, strerror(errno))
        return nullptr;
    }
    new_socket->m_ctx = m_ctx;
    if (new_socket->Init(new_raw_socket)) {
        return new_socket;
    }
    return nullptr;
}

auto SslSocket::Connect(const Address::ptr &address, uint64_t timeout) -> bool {
    if (!Socket::Connect(address, timeout)) {
        return false;
    }
    m_ctx.reset(SSL_CTX_new(SSLv23_client_method()), SSL_CTX_free);
    m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
    SSL_set_fd(m_ssl.get(), m_socket);
    return SSL_connect(m_ssl.get()) == 1;
}

auto SslSocket::Reconnect(uint64_t timeout) -> bool {
    if (m_remote_address == nullptr) {
        LOG_ERROR(sys_logger, "reconnect m_remote_address is nullptr");
        return false;
    }
    return Connect(m_remote_address, timeout);
}

auto SslSocket::Close() -> bool { return Socket::Close(); }

auto SslSocket::Send(const void *buffer, size_t length, int flags) -> ssize_t {
    if (m_ssl == nullptr) {
        return -1;
    }
    return SSL_write(m_ssl.get(), buffer, static_cast<int>(length));
}

auto SslSocket::SendIovec(iovec *buffers, size_t length, int flags) -> ssize_t {
    if (m_ssl == nullptr) {
        return -1;
    }
    int total = 0;
    for (auto i = 0; i < length; ++i) {
        int tmp =
            SSL_write(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
        if (tmp <= 0) {
            return tmp;
        }
        total += tmp;
        if (tmp != static_cast<int>(buffers[i].iov_len)) {
            break;
        }
    }
    return total;
}

auto SslSocket::SendTo(const void *buffer, size_t length,
                       const Address::ptr &to, int flags) -> ssize_t {
    return -1;
}

auto SslSocket::SendIovecTo(iovec *buffers, size_t length,
                            const Address::ptr &to, int flags) -> ssize_t {
    return -1;
}

auto SslSocket::Recv(void *buffer, size_t length, int flags) -> ssize_t {
    if (m_ssl == nullptr) {
        return -1;
    }
    return SSL_read(m_ssl.get(), buffer, length);
}

auto SslSocket::RecvIovec(iovec *buffers, size_t length, int flags) -> ssize_t {
    if (m_ssl == nullptr) {
        return -1;
    }
    int total = 0;
    for (auto i = 0; i < length; ++i) {
        int tmp =
            SSL_read(m_ssl.get(), buffers[i].iov_base, buffers[i].iov_len);
        if (tmp <= 0) {
            return tmp;
        }
        total += tmp;
        if (tmp != static_cast<int>(buffers[i].iov_len)) {
            break;
        }
    }
    return total;
}

auto SslSocket::RecvFrom(void *buffer, size_t length, const Address::ptr &from,
                         int flags) -> ssize_t {
    return -1;
}

auto SslSocket::RecvIovecFrom(iovec *buffers, size_t length,
                              const Address::ptr &from, int flags) -> ssize_t {
    return -1;
}

auto SslSocket::Dump(std::ostream &os) const -> std::ostream & {
    os << "[SSLSocket sock=" << m_socket << " is_connected=" << m_is_connected
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

auto SslSocket::ToString() const -> std::string {
    std::stringstream ss;
    Dump(ss);
    return ss.str();
}

auto SslSocket::LoadCertificates(const std::string &cert_file,
                                 const std::string &key_file) -> bool {
    m_ctx.reset(SSL_CTX_new(SSLv23_server_method()), SSL_CTX_free);
    if (SSL_CTX_use_certificate_chain_file(m_ctx.get(), cert_file.c_str()) !=
        1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "SSL_CTX_use_certificate_chain_file(%s) error",
                         cert_file.c_str())
        return false;
    }
    if (SSL_CTX_use_PrivateKey_file(m_ctx.get(), key_file.c_str(),
                                    SSL_FILETYPE_PEM) != 1) {
        LOG_CUSTOM_ERROR(sys_logger, "SSL_CTX_use_PrivateKey_file(%s) error",
                         cert_file.c_str())
        return false;
    }
    if (SSL_CTX_check_private_key(m_ctx.get()) != 1) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "SSL_CTX_check_private_key cert_file=%s, key_file=%s",
                         cert_file.c_str(), key_file.c_str())
        return false;
    }
    return true;
}

auto SslSocket::Init(int sock) -> bool {
    if (!Socket::Init(sock)) {
        return false;
    }
    m_ssl.reset(SSL_new(m_ctx.get()), SSL_free);
    SSL_set_fd(m_ssl.get(), m_socket);
    return SSL_accept(m_ssl.get()) == 1;
}
}  // namespace wtsclwq