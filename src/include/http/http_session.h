//
// Created by wtsclwq on 23-4-4.
//

#pragma once
#include "../socket/socket_stream.h"
#include "http_request.h"
#include "http_response.h"

namespace wtsclwq {

class HttpSession : public SocketStream {
  public:
    using ptr = std::shared_ptr<HttpSession>;

    HttpSession(const HttpSession& other) = delete;
    HttpSession(HttpSession&& other) = delete;
    auto operator=(const HttpSession& other) -> HttpSession& = delete;
    auto operator=(HttpSession&& other) -> HttpSession& = delete;
    HttpSession() = delete;

    explicit HttpSession(Socket::ptr socket, bool owner = true);

    /**
     * @brief 接受Http请求
     * @return
     */
    auto RecvRequest() -> HttpRequest::ptr;

    /**
     * @brief 发送Http相应
     * @return[in] res HTTP相应
     */
    auto SendResponse(const HttpResponse::ptr& response) -> int;
};

}  // namespace wtsclwq
