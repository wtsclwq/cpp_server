//
// Created by wtsclwq on 23-3-30.
//

#include "../include/socket/ipv6_address.h"

#include <memory>

#include "../include/log/log_manager.h"
#include "../include/util/net_util.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

IPv6Address::IPv6Address() { m_addr.sin6_family = AF_INET6; }

IPv6Address::IPv6Address(const sockaddr_in6& address) { m_addr = address; }

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = wtsclwq::GetBigEndianValue(port);
    // address是按字节存储的，没有转换字节序的必要
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

auto IPv6Address::Create(const char* address, uint16_t port)
    -> IPv6Address::ptr {
    IPv6Address::ptr res(new IPv6Address());
    res->m_addr.sin6_port = GetBigEndianValue(port);
    int flag = inet_pton(AF_INET6, address, &res->m_addr.sin6_addr);
    if (flag <= 0) {
        LOG_CUSTOM_ERROR(
            sys_logger,
            "IPv6Address::Create(%s, %d) flag = %d, errno = %d, errstr = %s",
            address, port, flag, errno, strerror(errno));
        return nullptr;
    }
    return res;
}

auto IPv6Address::GetAddr() -> sockaddr* {
    return reinterpret_cast<sockaddr*>(&m_addr);
}

auto IPv6Address::GetConstAddr() const -> const sockaddr* {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

auto IPv6Address::GetAddrLen() const -> socklen_t { return sizeof(m_addr); }

auto IPv6Address::Dump(std::ostream& os) const -> std::ostream& {
    os << "[";
    auto addr = (uint16_t*)m_addr.sin6_addr.s6_addr;  // NOLINT
    bool used_zeros = false;
    for (size_t i = 0; i < 8; ++i) {
        if (addr[i] == 0 && !used_zeros) {
            continue;
        }
        if (i != 0 && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if (i != 0) {
            os << ":";
        }
        os << std::hex << static_cast<int>(GetBigEndianValue(addr[i]))
           << std::dec;
    }
    if (!used_zeros && addr[7] == 0) {
        os << "::";
    }
    os << "]" << GetBigEndianValue(m_addr.sin6_port);
    return os;
}

auto IPv6Address::BroadcastAddress(uint32_t prefix_len) -> IPAddress::ptr {
    sockaddr_in6 broadcast_addr(m_addr);
    broadcast_addr.sin6_addr.s6_addr[prefix_len / 8] |=
        CreateMask<uint8_t>(prefix_len % 8);
    for (auto i = prefix_len / 8 + 1; i < 16; ++i) {
        broadcast_addr.sin6_addr.s6_addr[i] = 0xFF;
    }
    return std::make_shared<IPv6Address>(broadcast_addr);
}

auto IPv6Address::NetworkAddress(uint32_t prefix_len) -> IPAddress::ptr {
    sockaddr_in6 broadcast_addr(m_addr);
    broadcast_addr.sin6_addr.s6_addr[prefix_len / 8] &=
        ~CreateMask<uint8_t>(prefix_len % 8);
    for (auto i = prefix_len / 8 + 1; i < 16; ++i) {
        broadcast_addr.sin6_addr.s6_addr[i] = 0x00;
    }
    return std::make_shared<IPv6Address>(broadcast_addr);
}

auto IPv6Address::HostAddress(uint32_t prefix_len) -> IPAddress::ptr {
    sockaddr_in6 broadcast_addr(m_addr);
    broadcast_addr.sin6_addr.s6_addr[prefix_len / 8] &=
        CreateMask<uint8_t>(prefix_len % 8);
    for (auto i = prefix_len / 8 + 1; i < 16; ++i) {
        broadcast_addr.sin6_addr.s6_addr[i] = 0x00;
    }
    return std::make_shared<IPv6Address>(broadcast_addr);
}

auto IPv6Address::SubnetMask(uint32_t prefix_len) -> IPAddress::ptr {
    sockaddr_in6 host_mask{};
    host_mask.sin6_family = AF_INET6;
    host_mask.sin6_addr.s6_addr[prefix_len / 8] =
        ~CreateMask<uint8_t>(prefix_len % 8);
    for (auto i = 0; i < prefix_len / 8; ++i) {
        host_mask.sin6_addr.s6_addr[i] = 0xFF;
    }
    return std::make_shared<IPv6Address>(host_mask);
}

auto IPv6Address::GetPort() const -> uint16_t {
    return GetBigEndianValue(m_addr.sin6_port);
}

void IPv6Address::SetPort(uint16_t port) {
    m_addr.sin6_port = GetBigEndianValue(port);
}
}  // namespace wtsclwq