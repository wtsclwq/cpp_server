/*
 * @Description:
 * @LastEditTime: 2023-03-28 17:14:44
 */
#pragma once

#include <memory>

#include "../concurrency/lock.h"
#include "../util/singleton.h"
#include "io_manager.h"

namespace wtsclwq {
class FileDescriptor : public std::enable_shared_from_this<FileDescriptor> {
  public:
    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor(FileDescriptor &&) = delete;
    auto operator=(const FileDescriptor &) -> FileDescriptor & = delete;
    auto operator=(FileDescriptor &&) -> FileDescriptor & = delete;

    using ptr = std::shared_ptr<FileDescriptor>;
    explicit FileDescriptor(int filedesc);
    ~FileDescriptor() = default;

    auto Init() -> bool;
    [[maybe_unused]] auto IsInit() const -> bool;
    auto IsSocket() const -> bool;
    auto IsClosed() const -> bool;

    void SetUserNonBlock(bool val);
    auto GetUserNonBlock() const -> bool;

    [[maybe_unused]] void SetSystemNonBlock(bool val);
    auto GetSystemNonBlock() const -> bool;

    void SetTimeout(int type, uint64_t val);
    auto GetTimeout(int type) const -> uint64_t;

  private:
    bool m_is_init : 1;           // 是否初始化
    bool m_is_socket : 1;         // 是不是socket fd
    bool m_system_non_block : 1;  // 是否内核非阻塞
    bool m_user_non_block : 1;    // 是否用户非阻塞
    bool m_is_closed : 1;         // 是否关闭
    int m_fd;                     // 系统fd
    uint64_t m_recv_timeout;      // 接受超时时限
    uint64_t m_send_timeout;      // 发送超时时限
    wtsclwq::IOManager *m_iom;
};

/**
 * @brief FileDescriptorManagerImpl 文件描述符管理类
 */
class FileDescriptorManagerImpl {
  public:
    FileDescriptorManagerImpl();

    /**
     * @brief 获取文件描述符 fd 对应的包装对象，若指定参数 auto_create 为 true,
     * 当该包装对象不在管理类中时，自动创建一个新的包装对象
     * @return 返回指定的文件描述符的包装对象；当指定的文件描述符不存在时，返回
     * nullptr，如果指定 auto_create 为
     * true，则为这个文件描述符创建新的包装对象并返回。
     */
    auto Get(int filedesc, bool auto_create = false) -> FileDescriptor::ptr;

    /**
     * @brief 将一个文件描述符从管理类中删除
     */
    void Remove(int filedesc);

  private:
    RWLock m_lock{};
    std::vector<FileDescriptor::ptr> m_data;
};

using FileDescriptorManager = SingletonPtr<FileDescriptorManagerImpl>;

}  // namespace wtsclwq