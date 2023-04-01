//
// Created by wtsclwq on 23-3-30.
//

#include "../include/socket/ipv4_address.h"

#include <memory>

#include "../include/log/log_manager.h"
#include "../include/util/net_util.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

IPv4Address::IPv4Address() { m_addr.sin_family = AF_INET; }

IPv4Address::IPv4Address(const sockaddr_in& address) { m_addr = address; }

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    memset(&m_addr, 0, sizeof m_addr);
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = wtsclwq::GetBigEndianValue(port);            // local->net
    m_addr.sin_addr.s_addr = wtsclwq::GetBigEndianValue(address);  // local->net
}

auto IPv4Address::Create(const char* address, uint16_t port)
    -> IPv4Address::ptr {
    IPv4Address::ptr result(new IPv4Address());
    result->m_addr.sin_port = GetBigEndianValue(port);
    // 不需要转换字节序，inet_pton会自动进行
    int flag = inet_pton(AF_INET, address, &result->m_addr.sin_addr);
    if (flag <= 0) {
        LOG_CUSTOM_ERROR(
            sys_logger,
            "IPv4Address::Create(%s, %d) flag = %d, errno = %d, errstr = %s",
            address, port, flag, errno, strerror(errno));
        return nullptr;
    }
    return result;
}

auto IPv4Address::GetAddr() -> sockaddr* {
    return reinterpret_cast<sockaddr*>(&m_addr);
}

auto IPv4Address::GetConstAddr() const -> const sockaddr* {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

auto IPv4Address::GetAddrLen() const -> socklen_t { return sizeof(m_addr); }

auto IPv4Address::Dump(std::ostream& os) const -> std::ostream& {
    uint32_t addr = GetBigEndianValue(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xFF) << "," << ((addr >> 16) & 0xFF) << ","
       << ((addr >> 8) & 0xFF) << "," << (addr & 0xFF);
    os << ":" << GetBigEndianValue(m_addr.sin_port);
    return os;
}

auto IPv4Address::BroadcastAddress(uint32_t prefix_len) -> IPAddress::ptr {
    if (prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in broadcast_addr(m_addr);
    broadcast_addr.sin_addr.s_addr |=
        wtsclwq::GetBigEndianValue(wtsclwq::CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(broadcast_addr);
}

auto IPv4Address::NetworkAddress(uint32_t prefix_len) -> IPAddress::ptr {
    if (prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in network_addr(m_addr);
    network_addr.sin_addr.s_addr &=
        ~wtsclwq::GetBigEndianValue(wtsclwq::CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(network_addr);
}

auto IPv4Address::HostAddress(uint32_t prefix_len) -> IPAddress::ptr {
    if (prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in host_addr(m_addr);
    host_addr.sin_addr.s_addr &=
        wtsclwq::GetBigEndianValue(wtsclwq::CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(host_addr);
}

auto IPv4Address::SubnetMask(uint32_t prefix_len) -> IPAddress::ptr {
    if (prefix_len > 32) {
        return nullptr;
    }
    sockaddr_in subnet_addr{};
    subnet_addr.sin_family = AF_INET;
    subnet_addr.sin_addr.s_addr =
        ~wtsclwq::GetBigEndianValue(wtsclwq::CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(subnet_addr);
}

auto IPv4Address::GetPort() const -> uint16_t {
    return wtsclwq::GetBigEndianValue(m_addr.sin_port);
}
void IPv4Address::SetPort(uint16_t port) {
    m_addr.sin_port = wtsclwq::GetBigEndianValue(port);
}
}  // namespace wtsclwq