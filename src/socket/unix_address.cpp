//
// Created by wtsclwq on 23-3-30.
//

#include "../include/socket/unix_address.h"

#include "../include/log/log_manager.h"

namespace wtsclwq {

static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

static const size_t MAX_PATH_LEN =
    sizeof((reinterpret_cast<sockaddr_un*>(0))->sun_path) - 1;

UnixAddress::UnixAddress() {
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path) {
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    if (!path.empty() && path[0] == '\0') {
        --m_length;
    }
    if (m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

auto UnixAddress::GetAddr() -> sockaddr* {
    return reinterpret_cast<sockaddr*>(&m_addr);
}

auto UnixAddress::GetConstAddr() const -> const sockaddr* {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

auto UnixAddress::GetAddrLen() const -> socklen_t { return m_length; }

auto UnixAddress::Insert(std::ostream& os) const -> std::ostream& {
    if (m_length > offsetof(sockaddr_un, sun_path) &&
        m_addr.sun_path[0] == '0') {
        return os << "\\0"
                  << std::string(
                         m_addr.sun_path + 1,
                         m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}
}  // namespace wtsclwq