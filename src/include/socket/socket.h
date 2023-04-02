//
// Created by wtsclwq on 23-3-29.
//
#pragma once

#include <sys/socket.h>
#include <sys/types.h>

#include <memory>

#include "address.h"
namespace wtsclwq {

class Socket : public std::enable_shared_from_this<Socket> {
  public:
    using ptr = std::shared_ptr<Socket>;
    using weak_ptr = std::weak_ptr<Socket>;

    enum SocketType { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

    enum SocketFamily { IPV4 = AF_INET, IPV6 = AF_INET6, UNIX = AF_UNIX };

    Socket(const Socket& other) = delete;
    Socket(Socket&& other) = delete;
    auto operator=(const Socket& other) = delete;
    auto operator=(Socket&& other) = delete;
    Socket() = default;
    virtual ~Socket();

    Socket(int family, int type, int protocol = 0);

    /**
     * @brief 静态方法：创建一个TCP socket（满足地址类型）
     * @param[in] address
     * @return 成功则返回一个TCPSocket的智能指针，失败则返回nullptr
     */
    static auto CreateTcpSocket(const Address::ptr& address) -> Socket::ptr;

    /**
     * @brief 静态方法：UDP socket（满足地址类型）
     * @param[in] address
     * @return 成功则返回一个UDPSocket的智能指针，失败则返回nullptr
     */
    static auto CreateUdpSocket(const Address::ptr& address) -> Socket::ptr;

    /**
     * @brief 静态方法：创建IPV4的TCP Socket
     */
    static auto CreateIpv4TcpSocket() -> Socket::ptr;

    /**
     * @brief 静态方法：创建IPV4的UDP Socket
     */
    static auto CreateIpv4UdpSocket() -> Socket::ptr;

    /**
     * @brief 静态方法：创建IPV6的TCP Socket
     */
    static auto CreateIpv6TcpSocket() -> Socket::ptr;

    /**
     * @brief 静态方法：创建IPV6的UDP Socket
     */
    static auto CreateIpv6UdpSocket() -> Socket::ptr;

    /**
     * @brief 静态方法：创建Unix的TCP Socket
     */
    static auto CreateUnixTcpSocket() -> Socket::ptr;

    /**
     * @brief 静态方法：创建Unix的UDP Socket
     */
    static auto CreateUnixUdpSocket() -> Socket::ptr;

    /**
     * @brief 静态方法：获取发送超时时间(ms)
     */
    auto GetSendTimeout() const -> uint64_t;

    /**
     * @brief 静态方法：设置发送超时时间(ms)
     */
    void SetSendTimeout(uint64_t timeout);

    /**
     * @brief 静态方法：获取接收超时时间(ms)
     */
    auto GetRecvTimeout() const -> uint64_t;

    /**
     * @brief 成员方法：设置接收超时时间(ms)
     */
    void SetRecvTimeout(uint64_t timeout);

    /**
     * @brief 模板成员方法：获取socket option @see man getsockopt()
     */
    template <class T>
    auto GetOption(int level, int option, T& result) const -> bool;

    /**
     * @brief 成员方法：获取socket option @see man getsockopt()
     */
    auto GetOptionWithLen(int level, int option, void* result,
                          socklen_t* len) const -> bool;

    /**
     * @brief 模板成员方法:设置socket option @see man setsockopt()
     */
    template <class T>
    auto SetOption(int level, int option, const T& value) -> bool;

    /**
     * @brief 成员方法：设置socket option @see man setsockopt()
     */
    auto SetOptionWithLen(int level, int option, const void* result,
                          socklen_t socklen) -> bool;

    /**
     * @brief 虚成员方法：绑定Socket和Address @see man bind()
     * @prama[in] 要绑定到this的地址信息
     * @return 是否成功
     */
    virtual auto Bind(const Address::ptr& address) -> bool;

    /**
     * @brief 虚成员方法：监听连接请求 @see man listen()
     * @param[in] backlog 未完成连接队列的最大长度
     * @return 是否成功
     * @pre 必须先bind成功
     */
    virtual auto Listen(int backlog) -> bool;

    /**
     * @brief 虚成员方法：接受connect连接 @see man accept()
     * @return 成功则返回用于处理连接的Socket智能指针，失败则返回nullptr
     * @pre Socket必须Bind, Listen成功
     */
    virtual auto Accept() -> Socket::ptr;

    /**
     * @brief 虚成员函数：向目标地址尝试建立连接 @see man connect()
     * @param[in] address 目标地址
     * @param[in] timeout 超时时间(ms)
     * @return 是否成功
     */
    virtual auto Connect(const Address::ptr& address, uint64_t timeout) -> bool;

    /**
     * @brief 虚成员函数：尝试重新连接
     * @param[in] timeout 超时时间(ms)
     * @return 是否成功
     */
    virtual auto Reconnect(uint64_t timeout) -> bool;

    /**
     * @brief 关闭this @this man close()
     * @return 是否成功
     */
    virtual auto Close() -> bool;

    /**
     * @brief 虚成员函数：向this发送数据（TCP）
     * @param[in] buffer 待发送的数据缓冲
     * @param[in] length 待发送的数据长度
     * @param[in] flags  标识字
     * @return
     *      @retval >0 发送成功[对应大小]的数据
     *      @retval =0 this socket被关闭
     *      @retval <0 this socket错误
     */
    virtual auto Send(const void* buffer, size_t length, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：向this发送数据（TCP）
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送的数据长度(iovec数组长度)
     * @param[in] flags  标识字
     * @return
     *      @retval >0 发送成功[对应大小]的数据
     *      @retval =0 this socket被关闭
     *      @retval <0 this socket错误
     */
    virtual auto SendIovec(iovec* buffers, size_t length, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：向目标地址发送数据（UDP）
     * @param[in] buffer 待发送数据
     * @param[in] length 待发送数据长度
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual auto SendTo(const void* buffer, size_t length,
                        const Address::ptr& to, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：向目标地址发送数据（UDP）
     * @param[in] buffers 待发送数据的内存(iovec数组)
     * @param[in] length 待发送数据的长度(iovec长度)
     * @param[in] to 发送的目标地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 发送成功对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual auto SendIovecTo(iovec* buffers, size_t length,
                             const Address::ptr& to, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：this从缓冲接受数据（TCP）
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual auto Recv(void* buffer, size_t length, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：this从缓冲接受数据（TCP）
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual auto RecvIovec(iovec* buffers, size_t length, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：从指定地址接受数据（UDP）
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual auto RecvFrom(void* buffer, size_t length, const Address::ptr& from,
                          int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：从指定地址接受数据（UDP）
     * @param[out] buffers 接收数据的内存(iovec数组)
     * @param[in] length 接收数据的内存大小(iovec数组长度)
     * @param[out] from 发送端地址
     * @param[in] flags 标志字
     * @return
     *      @retval >0 接收到对应大小的数据
     *      @retval =0 socket被关闭
     *      @retval <0 socket出错
     */
    virtual auto RecvIovecFrom(iovec* buffers, size_t length,
                               const Address::ptr& from, int flags) -> ssize_t;

    /**
     * @brief 虚成员函数：将信息输入到流中
     */
    virtual auto Dump(std::ostream& os) const -> std::ostream&;

    /**
     * @brief 虚成员函数：将信息转换为可读字符串
     */
    virtual auto ToString() const -> std::string;

    /**
     * @brief 成员函数：获取远端地址
     */
    auto GetRemoteAddress() -> Address::ptr;

    /**
     * @brief 成员函数：获取本地地址
     */
    auto GetLocalAddress() -> Address::ptr;

    /**
     * @brief 成员函数：获取socket fd
     */
    auto GetSocket() const -> int;

    /**
     * @brief 成员函数：获取协议簇
     */
    auto GetFamily() const -> int;

    /**
     * @brief 成员函数：获取类型
     */
    auto GetType() const -> int;

    /**
     * @brief 成员函数：返回协议
     */
    auto GetProtocol() const -> int;

    /**
     * @brief 成员函数：返回是否已经连接
     */
    auto IsConnected() const -> bool;

    /**
     * @brief 成员函数：返回是否有效(m_sock != -1)
     * @return
     */
    auto IsValid() const -> bool;

    /**
     * @brief 成员函数：返回Socket错误
     */
    auto GetError() const -> int;

    /**
     * @brief 成员函数：取消读
     */
    auto CancelRead() -> bool;

    /**
     * @brief 成员函数：取消写
     */
    auto CancelWrite() -> bool;

    /**
     * @brief 成员函数：取消accept
     */
    auto CancelAccept() -> bool;

    /**
     * @brief 成员函数：取消所有动作
     */
    auto CancelAll() -> bool;

  protected:
    /**
     * @brief 成员函数：初始化socket
     */
    void InitSocket();

    /**
     * @brief 成员函数：创建socket
     */
    void NewSocket();

    /**
     * @brief 虚成员函数：初始化一个Socket，用于accept时创建的新socket
     */
    virtual auto Init(int sock) -> bool;

  protected:
    int m_socket{-1};                 // socket fd
    int m_family{};                   // 协议簇
    int m_type{};                     // 类型
    int m_protocol{};                 // 协议
    bool m_is_connected{false};       // 是否已连接
    Address::ptr m_local_address{};   // 本地地址
    Address::ptr m_remote_address{};  // 远端地址
};

template <class T>
auto Socket::GetOption(int level, int option, T& result) const -> bool {
    socklen_t length = sizeof(T);
    return GetOptionWithLen(level, option, &result, &length);
}

template <class T>
auto Socket::SetOption(int level, int option, const T& value) -> bool {
    return SetOptionWithLen(level, option, &value, sizeof(T));
}

}  // namespace wtsclwq
