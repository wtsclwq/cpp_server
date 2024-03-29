//
// Created by wtsclwq on 23-3-30.
//

#pragma once
#include "ip_address.h"
namespace wtsclwq {

class IPv4Address : public IPAddress {
  public:
    using ptr = std::shared_ptr<IPv4Address>;

    IPv4Address(const IPv4Address& other) = delete;
    IPv4Address(IPv4Address&& other) = delete;
    auto operator=(const IPv4Address& other) = delete;
    auto operator=(IPv4Address&& other) = delete;
    ~IPv4Address() override = default;

    IPv4Address();
    /**
     * @brief 构造函数：通过sockaddr_in构造对象
     * @param[in] address sockaddr_in引用
     */
    explicit IPv4Address(const sockaddr_in& address);

    /**
     * @brief 构造函数：通过二进制地址构造对象
     * @param address 二进制32位地址
     * @param port 端口号
     */
    explicit IPv4Address(uint32_t address, uint16_t port);

    /**
     * @brief 静态函数：创建点分十进制地址对应的IPv4Address智能指针
     * @param address 十进制地址如：127.0.0.1
     * @param port 端口号
     * @return 成功则返回IPAddress智能指针，失败则返回nullptr
     */
    static auto Create(const char* address, uint16_t port = 0)
        -> IPv4Address::ptr;
    /**
     * @brief 重写的成员函数：获取当前对象对应的sockaddr指针
     */
    auto GetAddr() -> sockaddr* override;
    /**
     * @brief 获取当前对象的const sockaddr指针
     */
    auto GetConstAddr() const -> const sockaddr* override;

    /**
     * @brief 重写的成员函数：获取当前对象的sockaddr长度
     */
    auto GetAddrLen() const -> socklen_t override;

    /**
     * @brief 重写的成员函数：将可读的地址的写入到stream中
     * @param[in] os 要写入的stream
     * @return 写入后的stream
     */
    auto Dump(std::ostream& os) const -> std::ostream& override;

    /**
     * @brief 重写的成员函数：获取当前对象的广播地址的IPAddress智能指针
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回广播ip的IPAddress智能指针，失败则返回nullptr
     */
    auto BroadcastAddress(uint32_t prefix_len) -> IPAddress::ptr override;

    /**
     * @brief 重写的成员函数：获取当前对象网络号的IPAddress智能指针
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回网络号的IPAddress智能指针，失败则返回nullptr
     */

    auto NetworkAddress(uint32_t prefix_len) -> IPAddress::ptr override;

    /**
     * @brief 重写的成员函数：获取当前对象主机号的IPAddress智能指针
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回网络号的IPAddress智能指针，失败则返回nullptr
     */
    auto HostAddress(uint32_t prefix_len) -> IPAddress::ptr override;

    /**
     * @brief 重写的成员函数：获取当前对象主机掩码的IPAddress智能指针
     * @param[in] prefix_len 子网掩码位数
     * @return 成功则返回子网掩码的IPAddress智能指针，失败则返回nullptr
     */
    auto SubnetMask(uint32_t prefix_len) -> IPAddress::ptr override;

    /**
     * @brief 重写的成员函数：获取当前对象的端口号
     */
    auto GetPort() const -> uint16_t override;

    /**
     * @brief 重写的成员函数：设置当前对象的端口号
     */
    void SetPort(uint16_t port) override;

  private:
    sockaddr_in m_addr{};  // 存储ipv4地址
};
}  // namespace wtsclwq
