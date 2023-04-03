//
// Created by wtsclwq on 23-4-3.
//

#pragma once
#include <functional>
#include <memory>

#include "../io/io_manager.h"
#include "../socket/socket.h"

namespace wtsclwq {

class TcpServer : public std::enable_shared_from_this<TcpServer> {
  public:
    using ptr = std::shared_ptr<TcpServer>;

    TcpServer(const TcpServer &other) = delete;
    TcpServer(TcpServer &&other) = delete;
    auto operator=(const TcpServer &other) = delete;
    auto operator=(TcpServer &&other) = delete;

    TcpServer();

    TcpServer(IOManager *worker, IOManager *acceptor);

    ~TcpServer();

    /**
     * @brief 服务器socket绑定一个地址
     * @param address 要绑定的地址
     * @return 是否成功
     */
    virtual auto Bind(const Address::ptr &address) -> bool;

    /**
     * @brief 绑定多个地址
     * @param[in] address_vec 需要绑定的地址数组
     * @param[out] fails_vec 绑定失败的地址数组
     * @return 是否成功
     */
    virtual auto BindVec(const std::vector<Address::ptr> &address_vec,
                         std::vector<Address::ptr> &fails_vec) -> bool;
    /**
     * @brief 启动服务
     * @return 是否成功
     * @pre 必须Bind()成功
     */
    virtual auto Start() -> bool;

    /**
     * @brief 停止服务
     */
    virtual void Stop();

    /**
     * @brief 返回可读字符串
     * @param prefix 前缀
     */
    virtual auto ToString(const std::string &prefix) -> std::string;

    auto GetName() const -> const std::string &;

    void SetName(const std::string &name);

    auto GetType() const -> const std::string &;

    void SetType(const std::string &m_type);

    auto IsStop() const -> bool;

    void SetIsStop(bool m_is_stop);

    auto GetRecvTimeout() const -> uint64_t;

    void SetRecvTimeout(uint64_t m_recv_timeout);

  protected:
    /**
     * @brief 处理新建立的客户端socket
     * @param client 要被处理的socket
     */
    virtual void HandleClient(const Socket::ptr &client);

    /**
     * @brief 开始接受连接
     * @param socket 接受连接的socket
     */
    virtual void StartAccept(const Socket::ptr &socket);

  private:
    std::string m_name;     // 服务器名称
    std::string m_type;     // 服务器类型
    bool m_is_stop;         // 服务器是否停止
    IOManager *m_worker;    // 用来处理新建socket连接的调度器
    IOManager *m_acceptor;  // 用来处理服务端socket的连接请求的接收
    uint64_t m_recv_timeout;             // 接受超时时限
    std::vector<Socket::ptr> m_sockets;  // 被监听的socket数组
};

}  // namespace wtsclwq
