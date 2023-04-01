//
// Created by wtsclwq on 23-3-29.
//
#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace wtsclwq {

class Address {
  public:
    using ptr = std::shared_ptr<Address>;

    Address(const Address& other) = delete;
    Address(Address&& other) = delete;
    auto operator=(const Address& other) = delete;
    auto operator=(Address&& other) = delete;
    Address() = default;
    /**
     * @brief 虚析构函数
     */
    virtual ~Address() = default;

    /**
     * 通过sockaddr指针获得对应的Address智能指针
     * @param[in] addr sockaddr的指针
     * @param[int] addrlen sockaddr的长度
     * @return 返回和sockaddr对应的Address智能指针，失败则返回nullptr
     */
    static auto Create(const sockaddr* addr, socklen_t addrlen) -> Address::ptr;

    /**
     * @brief 通过host地址返回对应条件的所有Address智能指针集合
     * @param [out] result 保存满足条件的Address
     * @param [in] host 域名、服务器名等，例如www.baidu.com[:80]
     * @param [in] family 协议簇(AF_INET, AF_INET6, AF_UNIX)等
     * @param [in] type socket类型，(SOCK_STREAM, SOCK_DGRAM)等
     * @param [in] protocol  具体协议，IPPROOTR_TCP, IPPRPTP_UDP等
     * @return
     */
    static auto Lookup(std::vector<Address::ptr>& result,
                       const std::string& host, int family = AF_INET,
                       int type = 0, int protocol = 0) -> bool;

    /**
     * @brief 通过host地址返回对应条件的任意一个Address智能指针
     * @param [in] host 域名、服务器名等，例如www.baidu.com[:80]
     * @param [in] family 协议簇(AF_INET, AF_INET6, AF_UNIX)等
     * @param [in] type socket类型，(SOCK_STREAM, SOCK_DGRAM)等
     * @param [in] protocol  具体协议，IPPROOTR_TCP, IPPRPTP_UDP等
     * @return
     */
    static auto LookupAny(const std::string& host, int family = AF_INET,
                          int type = 0, int protocol = 0) -> Address::ptr;

    /**
     * @brief 根据协议簇返回本机所有网卡的<网卡名，地址，子网掩码位数>
     * @param[out] result 保存本机所有地址
     * @param[in] family 协议簇(AF_INET,AF_INET6)
     * @return 获取是否成功
     */
    static auto GetAllInterFaceAddress(
        std::multimap<std::string, std::pair<Address::ptr, uint32_t>>& result,
        int family = AF_INET) -> bool;

    /**
     * @brief 获取指定网卡的地址和子网掩码位数
     * @param[out] result_vec 保存指定网卡所有地址
     * @param[in] interface 网卡名称
     * @param[in] family 协议族(AF_INT, AF_INT6, AF_UNIX)
     * @return 是否成功
     */
    static auto GetSpecficInterFaceAddress(
        std::vector<std::pair<Address::ptr, uint32_t>>& result_vec,
        const std::string& interface, int family = AF_INET);

    /**
     * @brief 获取当前对象的协议簇
     */
    auto GetFamily() const -> int;

    /**
     * @brief 获取当前对象的sockaddr指针
     */
    virtual auto GetAddr() -> sockaddr* = 0;

    /**
     * @brief 获取当前对象的const sockaddr指针
     */
    virtual auto GetConstAddr() const -> const sockaddr* = 0;

    /**
     * @brief 获取当前对象的sockaddr长度
     */
    virtual auto GetAddrLen() const -> socklen_t = 0;

    /**
     * @brief 将可读的地址的写入到stream中
     * @return 写入了地址的流
     */
    virtual auto Dump(std::ostream& os) const -> std::ostream& = 0;

    /**
     * @brief 返回地址的可读字符串
     * @return
     */
    auto ToString() const -> std::string;

    /**
     * @brief 重载<运算符
     */
    auto operator<(const Address& rhs) const -> bool;

    /**
     * @brief 重载相等运算符
     */
    auto operator==(const Address& rhs) const -> bool;

    /**
     * @brief 重载不等运算符
     */
    auto operator!=(const Address& rhs) const -> bool;
};

/**
 * @brief
 * 采用全局重载方式，重载<<运算符，使得所有Address及其派生类对象都能输入到流中
 * @param os
 * @param addr
 * @return
 */
inline auto operator<<(std::ostream& os, const Address& addr) -> std::ostream& {
    return addr.Dump(os);
}
}  // namespace wtsclwq
