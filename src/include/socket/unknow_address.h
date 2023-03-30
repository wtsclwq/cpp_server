//
// Created by wtsclwq on 23-3-30.
//

#pragma once
#include "address.h"

namespace wtsclwq {

class UnKnowAddress : public Address {
  public:
    using ptr = std::shared_ptr<UnKnowAddress>;

    UnKnowAddress(const UnKnowAddress& other) = delete;
    UnKnowAddress(UnKnowAddress&& other) = delete;
    auto operator=(const UnKnowAddress& other) = delete;
    auto operator=(UnKnowAddress&& other) = delete;
    ~UnKnowAddress() override = default;

    /**
     * @brief 构造函数：根据地址簇构造未知地址
     * @param[in] family 地址簇
     */
    explicit UnKnowAddress(int family);

    /**
     * @brief 构造函数：根据sockaddr构造未知地址
     * @param addr sockaddr对象的引用
     */
    explicit UnKnowAddress(const sockaddr& addr);

    /**
     * @brief 重写的成员函数：获取当前对象的sockaddr指针
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
     * @brief 重写的成员函数：可读的地址信息写入到stream
     * @param os 要被写入的流
     * @return 写入之后的流
     */
    auto Insert(std::ostream& os) const -> std::ostream& override;

  private:
    sockaddr m_addr{};  // 存储未知地址
};

}  // namespace wtsclwq
