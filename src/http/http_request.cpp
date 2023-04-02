//
// Created by wtsclwq on 23-4-2.
//

#include "../include/http/http_request.h"

#include "../include/util/string_util.h"

namespace wtsclwq {
#define PARSE_PARAM(str, m, flag, trim)                                        \
    size_t pos = 0;                                                            \
    do {                                                                       \
        size_t last = pos;                                                     \
        pos = (str).find('=', pos);                                            \
        if (pos == std::string::npos) {                                        \
            break;                                                             \
        }                                                                      \
        size_t key = pos;                                                      \
        pos = (str).find(flag, pos);                                           \
        (m).insert(std::make_pair(trim((str).substr(last, key - last)),        \
                                  wtsclwq::StringUtil::UrlDecode(              \
                                      (str).substr(key + 1, pos - key - 1)))); \
        if (pos == std::string::npos) {                                        \
            break;                                                             \
        }                                                                      \
        ++pos;                                                                 \
    } while (true);

HttpRequest::HttpRequest()
    : m_method(HttpMethod::GET), m_version(0x11), m_is_auto_close(true),
      m_is_websocket(false), m_parser_param_flag(0), m_path("/") {}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : m_method(HttpMethod::GET), m_version(version), m_is_auto_close(close),
      m_is_websocket(false), m_parser_param_flag(0), m_path("/") {}

void HttpRequest::Init() {
    std::string conn = GetHeaderStrWithDef("connection");
    if (conn.empty()) {
        return;
    }
    m_is_auto_close = strcasecmp(conn.c_str(), "keep-alive") != 0;
}

void HttpRequest::InitParam() {
    InitQueryParam();
    InitBodyParam();
    InitCookies();
}

void HttpRequest::InitQueryParam() {
    // 如果query已经被解析过
    if (m_parser_param_flag & 0x1) {
        return;
    }
    PARSE_PARAM(m_query, m_params, '&', );
    m_parser_param_flag |= 0x1;
}

void HttpRequest::InitBodyParam() {
    // 如果param已经被解析过
    if (m_parser_param_flag & 0x2) {
        return;
    }
    std::string content_type = GetHeaderStrWithDef("content-type");
    // 查找application/x-www-from-urlencoded第一次出现的位置，没找到就是nullptr
    if (strcasestr(content_type.c_str(), "application/x-www-from-urlencoded") ==
        nullptr) {
        m_parser_param_flag |= 0x2;
        return;
    }
    PARSE_PARAM(m_body, m_params, '&', );
    m_parser_param_flag |= 0x2;
}

void HttpRequest::InitCookies() {
    if (m_parser_param_flag & 0x4) {
        return;
    }
    std::string cookie = GetHeaderStrWithDef("cookie");
    if (cookie.empty()) {
        m_parser_param_flag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, m_cookies, ';', wtsclwq::StringUtil::Trim);
    m_parser_param_flag |= 0x4;
}

auto HttpRequest::GetHeaderStrWithDef(const std::string& key,
                                      const std::string& def) -> std::string {
    auto iter = m_headers.find(key);
    return iter == m_headers.end() ? def : iter->second;
}

auto HttpRequest::GetParamStrWithDef(const std::string& key,
                                     const std::string& def) -> std::string {
    InitQueryParam();
    InitBodyParam();
    auto iter = m_params.find(key);
    return iter == m_params.end() ? def : iter->second;
}

auto HttpRequest::GetCookieStrWithDef(const std::string& key,
                                      const std::string& def) -> std::string {
    InitCookies();
    auto iter = m_cookies.find(key);
    return iter == m_cookies.end() ? def : iter->second;
}

void HttpRequest::SetHeader(const std::string& key, const std::string& val) {
    m_headers[key] = val;
}

void HttpRequest::SetParam(const std::string& key, const std::string& val) {
    m_params[key] = val;
}

void HttpRequest::SetCookie(const std::string& key, const std::string& val) {
    m_cookies[key] = val;
}

void HttpRequest::DelHeader(const std::string& key) { m_headers.erase(key); }

void HttpRequest::DelParam(const std::string& key) { m_params.erase(key); }

void HttpRequest::DelCookie(const std::string& key) { m_cookies.erase(key); }

auto HttpRequest::HashHeader(const std::string& key) -> bool {
    return m_headers.find(key) != m_headers.end();
}

auto HttpRequest::HashParam(const std::string& key) -> bool {
    return m_params.find(key) != m_params.end();
}

auto HttpRequest::HashCookie(const std::string& key) -> bool {
    return m_cookies.find(key) != m_cookies.end();
}

auto HttpRequest::CreateRespense() -> std::shared_ptr<HttpResponse> {
    return std::make_shared<HttpResponse>(GetVersion(), IsAutoClose());
}

auto HttpRequest::Dump(std::ostream& os) const -> std::ostream& {
    os << HttpMethodToString(m_method) << " " << m_path
       << (m_query.empty() ? "" : "?") << m_query
       << (m_fragment.empty() ? "" : "#") << m_fragment << " HTTP/"
       << ((uint32_t)(m_version >> 4)) << "." << ((uint32_t)(m_version & 0x0F))
       << "\r\n";
    if (!m_is_websocket) {
        os << "connection: " << (m_is_auto_close ? "close" : "keep-alive")
           << "\r\n";
    }
    for (auto& i : m_headers) {
        if (!m_is_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }

    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

auto HttpRequest::ToString() const -> std::string {
    std::stringstream ss;
    Dump(ss);
    return ss.str();
}

auto HttpRequest::GetMethod() const -> HttpMethod { return m_method; }

void HttpRequest::SetMethod(HttpMethod method) { m_method = method; }

auto HttpRequest::GetVersion() const -> uint8_t { return m_version; }

void HttpRequest::SetVersion(uint8_t version) { m_version = version; }

auto HttpRequest::IsAutoClose() const -> bool { return m_is_auto_close; }

void HttpRequest::SetIsAutoClose(bool is_auto_close) {
    m_is_auto_close = is_auto_close;
}

auto HttpRequest::IsWebsocket() const -> bool { return m_is_websocket; }

void HttpRequest::SetIsWebsocket(bool is_websocket) {
    m_is_websocket = is_websocket;
}

auto HttpRequest::GetPath() const -> const std::string& { return m_path; }

void HttpRequest::SetPath(const std::string& path) { m_path = path; }

auto HttpRequest::GetQuery() const -> const std::string& { return m_query; }

void HttpRequest::SetQuery(const std::string& query) { m_query = query; }

auto HttpRequest::GetFragment() const -> const std::string& {
    return m_fragment;
}

void HttpRequest::SetFragment(const std::string& fragment) {
    m_fragment = fragment;
}

auto HttpRequest::GetBody() const -> const std::string& { return m_body; }

void HttpRequest::SetBody(const std::string& body) { m_body = body; }

auto HttpRequest::GetHeaders() const -> const HttpRequest::MappingType& {
    return m_headers;
}

void HttpRequest::SetHeaders(const HttpRequest::MappingType& headers) {
    m_headers = headers;
}

auto HttpRequest::GetParams() const -> const HttpRequest::MappingType& {
    return m_params;
}

void HttpRequest::SetParams(const HttpRequest::MappingType& params) {
    m_params = params;
}

auto HttpRequest::GetCookies() const -> const HttpRequest::MappingType& {
    return m_cookies;
}

void HttpRequest::SetCookies(const HttpRequest::MappingType& cookies) {
    m_cookies = cookies;
}

auto operator<<(std::ostream& os, const HttpRequest& request) -> std::ostream& {
    return request.Dump(os);
}

}  // namespace wtsclwq