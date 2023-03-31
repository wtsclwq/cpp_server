//
// Created by wtsclwq on 23-3-30.
//

#pragma once
#include "address.h"
namespace wtsclwq {

class UnixAddress : public Address {
  public:
    using ptr = std::shared_ptr<UnixAddress>;

    UnixAddress(const UnixAddress& other) = delete;
    UnixAddress(UnixAddress&& other) = delete;
    auto operator=(const UnixAddress& other) = delete;
    auto operator=(UnixAddress&& other) = delete;
    ~UnixAddress() override = default;

    UnixAddress();

    /**
     * @brief 构造函数：通过路径构造UnixAddress
     * @param[in] address UnixSocket路径(长度小于UNIX_PATH_MAX)
     */
    explicit UnixAddress(const std::string& path);

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
     * @brief 成员函数：设置sockaddr_len
     */
    void SetAddrLen(socklen_t length);

    /**
     * @brief 成员函数：获取当前对象的路径
     */
    auto GetPath() const -> std::string;

    /**
     * @brief 重写的成员函数：将可读的地址的写入到stream中
     * @param[in] os 要写入的stream
     * @return 写入后的stream
     */
    auto Insert(std::ostream& os) const -> std::ostream& override;

  private:
    sockaddr_un m_addr{};   // 存储unix地址
    socklen_t m_length{0};  // 地址长度
};

}  // namespace wtsclwq
