//
// Created by wtsclwq on 23-4-2.
//

#include "../include/http/http_response.h"

#include "../include/http/http_status.h"
#include "../include/util/string_util.h"

namespace wtsclwq {

HttpResponse::HttpResponse()
    : m_status(HttpStatus::OK), m_version(0x11), m_is_auto_close(true),
      m_is_web_socket(false) {}

HttpResponse::HttpResponse(uint8_t version, bool close)
    : m_status(HttpStatus::OK), m_version(version), m_is_auto_close(close),
      m_is_web_socket(false) {}

auto HttpResponse::GetHeaderStrWithDef(const std::string &key,
                                       const std::string &def) const
    -> std::string {
    auto iter = m_headers.find(key);
    return iter == m_headers.end() ? def : iter->second;
}

void HttpResponse::SetHeader(const std::string &key, const std::string &val) {
    m_headers[key] = val;
}

void HttpResponse::DelHeader(const std::string &key) { m_headers.erase(key); }

void HttpResponse::SetRedirect(const std::string &uri) {
    m_status = HttpStatus::FOUND;
    SetHeader("Loocation", uri);
}

void HttpResponse::SetCookie(const std::string &key, const std::string &val,
                             time_t expired, const std::string &path,
                             const std::string &domain, bool secure) {
    std::stringstream ss;
    ss << key << "=" << val;
    if (expired > 0) {
        ss << ";expires="
           << StringUtil::TimeToStr(expired, "%a, %d %b %Y %H:%M:%S") << "GMT";
    }
    if (!domain.empty()) {
        ss << ";domain=" << domain;
    }
    if (!path.empty()) {
        ss << ";path=" << path;
    }
    if (secure) {
        ss << ";secure";
    }
    m_cookies.push_back(ss.str());
}

auto HttpResponse::Dump(std::ostream &os) -> std::ostream & {
    os << "HTTP/" << ((uint32_t)(m_version >> 4)) << "."
       << ((uint32_t)(m_version & 0x0F)) << " " << (uint32_t)m_status << " "
       << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason)
       << "\r\n";

    for (auto &i : m_headers) {
        if (!m_is_web_socket &&
            strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    for (auto &i : m_cookies) {
        os << "Set-Cookie: " << i << "\r\n";
    }
    if (!m_is_web_socket) {
        os << "connection: " << (m_is_auto_close ? "close" : "keep-alive")
           << "\r\n";
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

auto HttpResponse::ToString() -> std::string {
    std::stringstream ss;
    Dump(ss);
    return ss.str();
}

auto HttpResponse::GetStatus() const -> HttpStatus { return m_status; }

void HttpResponse::SetStatus(HttpStatus status) { m_status = status; }

auto HttpResponse::GetVersion() const -> uint8_t { return m_version; }

void HttpResponse::SetVersion(uint8_t version) { m_version = version; }

auto HttpResponse::IsAutoClose() const -> bool { return m_is_auto_close; }

void HttpResponse::SetIsAutoClose(bool is_auto_close) {
    m_is_auto_close = is_auto_close;
}

auto HttpResponse::IsWebSocket() const -> bool { return m_is_web_socket; }

void HttpResponse::SetIsWebSocket(bool is_web_socket) {
    m_is_web_socket = is_web_socket;
}

auto HttpResponse::GetBody() const -> const std::string & { return m_body; }

void HttpResponse::SetBody(const std::string &body) { m_body = body; }

auto HttpResponse::GetReason() const -> const std::string & { return m_reason; }

void HttpResponse::SetReason(const std::string &reason) { m_reason = reason; }

auto HttpResponse::GetHeaders() const -> const HttpResponse::MappingType & {
    return m_headers;
}

void HttpResponse::SetHeaders(const HttpResponse::MappingType &headers) {
    m_headers = headers;
}

auto HttpResponse::GetCookies() const -> const std::vector<std::string> & {
    return m_cookies;
}

void HttpResponse::SetCookies(const std::vector<std::string> &cookies) {
    m_cookies = cookies;
}
}  // namespace wtsclwq