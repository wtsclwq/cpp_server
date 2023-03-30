//
// Created by wtsclwq on 23-3-30.
//

#pragma once
#include "address.h"
namespace wtsclwq {

class IPAddress : public Address {
  public:
    using ptr = std::shared_ptr<IPAddress>;

    IPAddress(const IPAddress& other) = delete;
    IPAddress(IPAddress&& other) = delete;
    auto operator=(const IPAddress& other) = delete;
    auto operator=(IPAddress&& other) = delete;
    IPAddress() = default;
    ~IPAddress() override = default;
    /**
     * @brief 通过域名，IP,服务器名称创建对应的IpAddress智能指针
     * @param[in] address 域名，IP,服务器名等，例如www.baidu.com
     * @param[in] port 端口号
     * @return 创建成功返回智能指针，失败则返回nullptr
     */
    static auto Create(const char* address, uint16_t port = 0)
        -> IPAddress::ptr;

    /**
     * @brief 静态函数：通过host地址返回对应条件的任意IPAddress
     * @param[in] host 域名,服务器名等.举例: www.baidu.top[:80]
     * (方括号为可选内容)
     * @param[in] family 协议族(AF_INET, AF_INET6, AF_UNIX)
     * @param[in] type socket类型SOCK_STREAM、SOCK_DGRAM 等
     * @param[in] protocol 协议,IPPROTO_TCP、IPPROTO_UDP 等
     * @return 返回满足条件的任意IPAddress,失败返回nullptr
     */
    static auto LookupAnyAddress(const std::string& host, int family = AF_INET,
                                 int type = 0, int protocol = 0)
        -> IPAddress::ptr;

    /**
     * @brief 纯虚函数：获取当前对象的广播ip地址,
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回广播地址对应的IPAddress智能指针，失败则返回nullptr
     */
    virtual auto BroadcastAddress(uint32_t prefix_len) -> IPAddress::ptr = 0;

    /**
     * @brief 纯虚函数：获取当前对象的网络号
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回网络号对应的IPAddress智能指针，失败则返回nullptr
     */
    virtual auto NetworkAddress(uint32_t prefix_len) -> IPAddress::ptr = 0;

    /**
     * @brief 纯虚函数：获取当前对象的主机号
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回网络号对应的IPAddress智能指针，失败则返回nullptr
     */
    virtual auto HostAddress(uint32_t prefix_len) -> IPAddress::ptr = 0;

    /**
     * @brief 纯虚函数：获取当前对象的子网掩码，即11111.....000
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回子网掩码对应的IPAddress智能指针，失败则返回nullptr
     */
    virtual auto SubnetMask(uint32_t prefix_len) -> IPAddress::ptr = 0;

    /**
     * @brief 纯虚函数：获取当前对象的端口号
     */
    virtual auto GetPort() const -> uint16_t = 0;

    /**
     * @brief 纯虚函数：设置当前对象的端口号
     */
    virtual void SetPort(uint16_t port) = 0;
};

}  // namespace wtsclwq
