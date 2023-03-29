//
// Created by wtsclwq on 23-3-29.
//
#pragma once

#include <sys/socket.h>
#include <sys/types.h>

#include <memory>
namespace wtsclwq {

class Socket : public std::enable_shared_from_this<Socket> {
  public:
    Socket(const Socket& other) = delete;
    Socket(Socket&& other) = delete;
    auto operator=(const Socket& other) = delete;
    auto operator=(Socket&& other) = delete;

    using ptr = std::shared_ptr<Socket>;
    using weak_ptr = std::weak_ptr<Socket>;

    enum SocketType { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

    enum SocketFamily { IPV4 = AF_INET, IPV6 = AF_INET6, UINX = AF_UNIX };
};
}  // namespace wtsclwq
