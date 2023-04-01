//
// Created by wtsclwq on 23-3-29.
//

#include "../include/socket/address.h"

#include <ifaddrs.h>
#include <libnet.h>

#include "../include/log/log_manager.h"
#include "../include/socket/ipv4_address.h"
#include "../include/socket/ipv6_address.h"
#include "../include/socket/unix_address.h"
#include "../include/socket/unknow_address.h"
#include "../include/util/net_util.h"

namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

auto Address::Create(const sockaddr *addr, socklen_t addrlen) -> Address::ptr {
    if (addr == nullptr) {
        return nullptr;
    }
    Address::ptr result{};
    switch (addr->sa_family) {
        case AF_INET:
            result.reset(
                new IPv4Address(*reinterpret_cast<const sockaddr_in *>(addr)));
            break;
        case AF_INET6:
            result.reset(
                new IPv6Address(*reinterpret_cast<const sockaddr_in6 *>(addr)));
            break;
        default:
            result.reset(new UnKnowAddress(*addr));
            break;
    }
    return result;
}

auto Address::Lookup(std::vector<Address::ptr> &result, const std::string &host,
                     int family, int type, int protocol) -> bool {
    addrinfo hints{};
    addrinfo *results;
    addrinfo *next;

    // 指定要获取的额外信息和偏好
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;

    std::string node{};
    const char *service = nullptr;
    // 检查ipv6address service
    if (!host.empty() && host[0] == '[') {
        const char *end_ipv6 =
            static_cast<const char *>(memchr(host.c_str(), ']', host.size()));
        if (end_ipv6 != nullptr) {
            if (*(end_ipv6 + 1) == ':') {
                service = end_ipv6 + 2;
            }
            node = host.substr(1, end_ipv6 - host.c_str() - 1);
            LOG_INFO(sys_logger, "Address::Lookup ipv6 node = " + node);
        }
    }
    // 没有[]包裹
    if (node.empty()) {
        node = host;
    }

    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if (error != 0) {
        LOG_CUSTOM_ERROR(
            sys_logger,
            "Address::Lookup getaddress(%s, %d, %d) err = %d, errstr = %s",
            host.c_str(), family, type, error, gai_strerror(error));
    }

    // 同一个域名可能会包含多个服务器地址信息
    next = results;
    while (next != nullptr) {
        result.push_back(
            Create(next->ai_addr, static_cast<socklen_t>(next->ai_addrlen)));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    return !result.empty();
}

auto Address::LookupAny(const std::string &host, int family, int type,
                        int protocol) -> Address::ptr {
    std::vector<Address::ptr> result;
    // 如果能解析到地址信息，则直接返回第一个
    if (Lookup(result, host, family, type, protocol)) {
        return result[0];
    }
    return nullptr;
}

auto Address::GetAllInterFaceAddress(
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result,
    int family) -> bool {
    struct ifaddrs *next;
    struct ifaddrs *results;

    // 将网卡地址信息存到results
    if (getifaddrs(&results) != 0) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "Address::GetAllInterFaceAddress getifaddrs() err = "
                         "%d, errstr = %s",
                         errno, strerror(errno));
        return false;
    }
    try {
        // 可能有多个网卡信息
        for (next = results; next != nullptr; next = next->ifa_next) {
            Address::ptr addr;
            uint32_t prefix_len = ~0U;
            // 如果不是 指定了family且指定family与目前遍历的对象不相符
            if (family != AF_UNSPEC && family != next->ifa_addr->sa_family) {
                continue;
            }
            switch (next->ifa_addr->sa_family) {
                case AF_INET: {
                    addr = Address::Create(next->ifa_addr, sizeof(sockaddr_in));
                    uint32_t v4_netmask =
                        reinterpret_cast<sockaddr_in *>(next->ifa_netmask)
                            ->sin_addr.s_addr;
                    prefix_len = wtsclwq::CountBytes(v4_netmask);
                    break;
                }
                case AF_INET6: {
                    addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                    in6_addr &v6_netmask =
                        reinterpret_cast<sockaddr_in6 *>(next->ifa_netmask)
                            ->sin6_addr;
                    prefix_len = 0;
                    for (int i = 0; i < 16; ++i) {
                        prefix_len += CountBytes(v6_netmask.s6_addr[i]);
                    }
                    break;
                }
                default:
                    break;
            }

            if (addr) {
                result.insert(std::make_pair(next->ifa_name,
                                             std::make_pair(addr, prefix_len)));
            }
        }
    } catch (...) {
        LOG_ERROR(sys_logger, "Address::GetInterfaceAddresses exception");
        freeifaddrs(results);
        return false;
    }
    freeifaddrs(results);
    return !result.empty();
}

auto Address::GetSpecficInterFaceAddress(
    std::vector<std::pair<Address::ptr, uint32_t>> &result_vec,
    const std::string &interface, int family) {
    if (interface.empty() || interface == "*") {
        if (family == AF_INET || family == AF_UNSPEC) {
            result_vec.emplace_back(std::make_shared<IPv4Address>(), 0);
        }
        if (family == AF_INET6 || family == AF_UNSPEC) {
            result_vec.emplace_back(std::make_shared<IPv6Address>(), 0);
        }
        return true;
    }
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results_map;
    if (!GetAllInterFaceAddress(results_map, family)) {
        return false;
    }

    for (auto iter_pair = results_map.equal_range(interface);
         iter_pair.first != iter_pair.second; ++iter_pair.first) {
        result_vec.push_back(iter_pair.first->second);
    }
    return !result_vec.empty();
}

auto Address::GetFamily() const -> int { return GetConstAddr()->sa_family; }

auto Address::ToString() const -> std::string {
    std::stringstream ss{};
    // 每个派生类是需要重写Insert函数即可
    Dump(ss);
    return ss.str();
}

auto Address::operator<(const Address &rhs) const -> bool {
    socklen_t min_len = std::min(GetAddrLen(), rhs.GetAddrLen());
    int result = memcmp(GetConstAddr(), rhs.GetConstAddr(), min_len);
    if (result < 0) {
        return true;
    }
    if (result > 0) {
        return false;
    }
    if (GetAddrLen() < rhs.GetAddrLen()) {
        return true;
    }
    return false;
}
auto Address::operator==(const Address &rhs) const -> bool {
    return GetAddrLen() == rhs.GetAddrLen() &&
           memcmp(GetConstAddr(), rhs.GetConstAddr(), GetAddrLen()) == 0;
}
auto Address::operator!=(const Address &rhs) const -> bool {
    return !(*this == rhs);
}
}  // namespace wtsclwq