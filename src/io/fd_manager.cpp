/*
 * @Description:
 * @LastEditTime: 2023-03-29 18:12:25
 */
#include "../include/io/fd_manager.h"

#include <asm-generic/socket.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../include/io/hook.h"
namespace wtsclwq {

FileDescriptor::FileDescriptor(int filedesc)
    : m_is_init(false), m_is_socket(false), m_system_non_block(false),
      m_user_non_block(false), m_is_closed(false), m_fd(filedesc),
      m_recv_timeout(UINT64_MAX), m_send_timeout(UINT64_MAX), m_iom(nullptr) {
    Init();
}

auto FileDescriptor::Init() -> bool {
    if (m_is_init) {
        return true;
    }
    m_recv_timeout = UINT64_MAX;
    m_send_timeout = UINT64_MAX;
    struct stat fd_stat {};
    if (fstat(m_fd, &fd_stat) == -1) {
        m_is_init = false;
        m_is_socket = false;
    } else {
        m_is_init = true;
        m_is_socket = S_ISSOCK(fd_stat.st_mode);  // NOLINT
    }
    if (m_is_socket) {
        int flags = fcntl_f(m_fd, F_GETFL, 0);
        if (!(flags & O_NONBLOCK)) {                     // NOLINT
            fcntl_f(m_fd, F_SETFL, flags | O_NONBLOCK);  // NOLINT
        }
        m_system_non_block = true;
    } else {
        m_system_non_block = false;
    }
    m_user_non_block = false;
    m_is_closed = false;
    return m_is_init;
}

[[maybe_unused]] auto FileDescriptor::IsInit() const -> bool { return m_is_init; }
auto FileDescriptor::IsSocket() const -> bool { return m_is_socket; }
auto FileDescriptor::IsClosed() const -> bool { return m_is_closed; }

void FileDescriptor::SetUserNonBlock(bool val) { m_user_non_block = val; }
auto FileDescriptor::GetUserNonBlock() const -> bool {
    return m_user_non_block;
}
[[maybe_unused]] void FileDescriptor::SetSystemNonBlock(bool val) { m_system_non_block = val; }
auto FileDescriptor::GetSystemNonBlock() const -> bool {
    return m_system_non_block;
}

void FileDescriptor::SetTimeout(int type, uint64_t val) {
    if (type == SO_RCVTIMEO) {
        m_recv_timeout = val;
    } else {
        m_send_timeout = val;
    }
}
auto FileDescriptor::GetTimeout(int type) const -> uint64_t {
    if (type == SO_RCVTIMEO) {
        return m_recv_timeout;
    }
    return m_send_timeout;
}

FileDescriptorManagerImpl::FileDescriptorManagerImpl() {
    const int manager_size = 64;
    m_data.resize(manager_size);
}

auto FileDescriptorManagerImpl::Get(int filedesc, bool auto_create)
    -> FileDescriptor::ptr {
    if (filedesc == -1) {
        return nullptr;
    }
    {
        ScopedReadLock lock(m_lock);
        if (static_cast<int>(m_data.size()) <= filedesc) {
            if (!auto_create) {
                return nullptr;
            }
        } else {
            if (m_data[filedesc] || !auto_create) {
                return m_data[filedesc];
            }
        }
    }
    ScopedWriteLock lock(m_lock);
    FileDescriptor::ptr fdp(new FileDescriptor(filedesc));
    if (filedesc >= static_cast<int>(m_data.size())) {
        m_data.resize(filedesc * 1.5);  // NOLINT
    }
    m_data[filedesc] = fdp;
    return fdp;
}

/**
 * @brief 将一个文件描述符从管理类中删除
 */
void FileDescriptorManagerImpl::Remove(int filedesc) {
    ScopedWriteLock lock(m_lock);
    if (static_cast<int>(m_data.size()) <= filedesc) {
        return;
    }
    m_data[filedesc].reset();
}

}  // namespace wtsclwq