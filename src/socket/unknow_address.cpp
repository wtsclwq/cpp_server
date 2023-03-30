//
// Created by wtsclwq on 23-3-30.
//

#include "../include/socket/unknow_address.h"

namespace wtsclwq {
UnKnowAddress::UnKnowAddress(int family) { m_addr.sa_family = family; }

UnKnowAddress::UnKnowAddress(const sockaddr& addr) { m_addr = addr; }

auto UnKnowAddress::GetAddr() -> sockaddr* { return &m_addr; }

auto UnKnowAddress::GetConstAddr() const -> const sockaddr* { return &m_addr; }

auto UnKnowAddress::GetAddrLen() const -> socklen_t { return sizeof(m_addr); }

auto UnKnowAddress::Insert(std::ostream& os) const -> std::ostream& {
    os << "[UnknownAddress family = " << m_addr.sa_family << "]";
    return os;
}
}  // namespace wtsclwq