//
// Created by wtsclwq on 23-4-4.
//

#include "../include/http/http_session.h"

#include "../include/http/http_parser.h"

namespace wtsclwq {
HttpSession::HttpSession(Socket::ptr socket, bool owner)
    : SocketStream(std::move(socket), owner) {}

auto HttpSession::RecvRequest() -> HttpRequest::ptr {
    HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    std::shared_ptr<char> buffer(new char[buff_size],
                                 [](const char *ptr) { delete[] ptr; });
    char *data = buffer.get();
    int offset = 0;
    do {
        // 从socket中读取到请求数据存到data中
        int len = SocketStream::ReadToBuf(data + offset, buff_size - offset);
        if (len <= 0) {
            SocketStream::Close();
            return nullptr;
        }
        len += offset;
        // 将请求数据拿去解析
        size_t nparse = parser->Execute(data, len);
        if (parser->HasError()) {
            SocketStream::Close();
            return nullptr;
        }
        offset = len - nparse;
        if (offset == buff_size) {
            SocketStream::Close();
            return nullptr;
        }
        // 解析完跳出
        if (parser->IsFinished()) {
            break;
        }
    } while (true);
    parser->GetData()->Init();
    return parser->GetData();
}

auto HttpSession::SendResponse(const HttpResponse::ptr &response) -> int {
    std::stringstream ss;
    ss << *response;
    std::string data = ss.str();
    return WriteFixSizeFromBuf(data.data(), data.size());
}
}  // namespace wtsclwq