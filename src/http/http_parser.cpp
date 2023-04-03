/**
 * @file http_parser.cc
 * @brief HTTP协议解析实现
 * @version 0.1
 * @date 2021-09-25
 */
#include "../include/http/http_parser.h"

#include "../include/config/config.h"
#include "../include/log/log_manager.h"

namespace wtsclwq {

static wtsclwq::Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

static wtsclwq::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    wtsclwq::Config::Lookup("http.request.buffer_size", (uint64_t)(4 * 1024),
                            "http request buffer size");

static wtsclwq::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    wtsclwq::Config::Lookup("http.request.max_body_size",
                            (uint64_t)(64 * 1024 * 1024),
                            "http request max body size");

static wtsclwq::ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
    wtsclwq::Config::Lookup("http.response.buffer_size", (uint64_t)(4 * 1024),
                            "http response buffer size");

static wtsclwq::ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
    wtsclwq::Config::Lookup("http.response.max_body_size",
                            (uint64_t)(64 * 1024 * 1024),
                            "http response max body size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;
static uint64_t s_http_response_buffer_size = 0;
static uint64_t s_http_response_max_body_size = 0;

auto HttpRequestParser::GetHttpRequestBufferSize() -> uint64_t {
    return s_http_request_buffer_size;
}

auto HttpRequestParser::GetHttpRequestMaxBodySize() -> uint64_t {
    return s_http_request_max_body_size;
}

auto HttpResponseParser::GetHttpResponseBufferSize() -> uint64_t {
    return s_http_response_buffer_size;
}

auto HttpResponseParser::GetHttpResponseMaxBodySize() -> uint64_t {
    return s_http_response_max_body_size;
}

namespace {
struct RequestSizeIniter {
    RequestSizeIniter() {
        s_http_request_buffer_size = g_http_request_buffer_size->GetValue();
        s_http_request_max_body_size = g_http_request_max_body_size->GetValue();
        s_http_response_buffer_size = g_http_response_buffer_size->GetValue();
        s_http_response_max_body_size =
            g_http_response_max_body_size->GetValue();

        g_http_request_buffer_size->AddListener(
            [](const uint64_t &ov, const uint64_t &nv) {
                s_http_request_buffer_size = nv;
            });

        g_http_request_max_body_size->AddListener(
            [](const uint64_t &ov, const uint64_t &nv) {
                s_http_request_max_body_size = nv;
            });

        g_http_response_buffer_size->AddListener(
            [](const uint64_t &ov, const uint64_t &nv) {
                s_http_response_buffer_size = nv;
            });

        g_http_response_max_body_size->AddListener(
            [](const uint64_t &ov, const uint64_t &nv) {
                s_http_response_max_body_size = nv;
            });
    }
};
__attribute__((unused)) static RequestSizeIniter init;
}  // namespace

/**
 * @brief http请求开始解析回调函数
 */
static auto OnRequestMessageBeginCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_request_message_begin_cb");
    return 0;
}

/**
 * @brief http请求头部字段解析结束，可获取头部信息字段，如method/version等
 * @note
 * 返回0表示成功，返回1表示该HTTP消息无消息体，返回2表示无消息体并且该连接后续不会再有消息
 */
static auto OnRequestHeadersCompleteCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_request_headers_complete_cb");
    auto *parser = static_cast<HttpRequestParser *>(p->data);
    parser->GetData()->SetVersion(((p->http_major) << 0x4) | (p->http_minor));
    parser->GetData()->SetMethod(static_cast<HttpMethod>(p->method));
    return 0;
}

/**
 * @brief http解析结束回调
 */
static auto OnRequestMessageCompleteCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_request_message_complete_cb");
    auto *parser = static_cast<HttpRequestParser *>(p->data);
    parser->SetFinished(true);
    return 0;
}

/**
 * @brief http分段头部回调，可获取分段长度
 */
static auto OnRequestChunkHeaderCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_request_chunk_header_cb");
    return 0;
}

/**
 * @brief http分段结束回调，表示当前分段已解析完成
 */
static auto OnRequestChunkCompleteCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_request_chunk_complete_cb");
    return 0;
}

/**
 * @brief http请求url解析完成回调
 */
static auto OnRequestUrlCb(http_parser *p, const char *buf, size_t len) -> int {
    LOG_CUSTOM_DEBUG(sys_logger, "on_request_url_cb, url is: %s",
                     std::string(buf, len).c_str())
    int ret;
    struct http_parser_url url_parser;
    auto *parser = static_cast<HttpRequestParser *>(p->data);

    http_parser_url_init(&url_parser);
    ret = http_parser_parse_url(buf, len, 0, &url_parser);
    if (ret != 0) {
        LOG_DEBUG(sys_logger, "parse url fail");
        return 1;
    }
    if (url_parser.field_set & (1 << UF_PATH)) {
        parser->GetData()->SetPath(
            std::string(buf + url_parser.field_data[UF_PATH].off,
                        url_parser.field_data[UF_PATH].len));
    }
    if (url_parser.field_set & (1 << UF_QUERY)) {
        parser->GetData()->SetQuery(
            std::string(buf + url_parser.field_data[UF_QUERY].off,
                        url_parser.field_data[UF_QUERY].len));
    }
    if (url_parser.field_set & (1 << UF_FRAGMENT)) {
        parser->GetData()->SetFragment(
            std::string(buf + url_parser.field_data[UF_FRAGMENT].off,
                        url_parser.field_data[UF_FRAGMENT].len));
    }
    return 0;
}

/**
 * @brief http请求首部字段名称解析完成回调
 */
static auto OnRequestHeaderFieldCb(http_parser *p, const char *buf, size_t len)
    -> int {
    std::string field(buf, len);
    LOG_CUSTOM_DEBUG(sys_logger, "on_request_header_field_cb, field is: %s",
                     field.c_str())
    auto *parser = static_cast<HttpRequestParser *>(p->data);
    parser->SetField(field);
    return 0;
}

/**
 * @brief http请求首部字段值解析完成回调
 */
static auto OnRequestHeaderValueCb(http_parser *p, const char *buf, size_t len)
    -> int {
    std::string value(buf, len);
    LOG_CUSTOM_DEBUG(sys_logger, "on_request_header_value_cb, value is: %s",
                     value.c_str())
    auto *parser = static_cast<HttpRequestParser *>(p->data);
    parser->GetData()->SetHeader(parser->GetField(), value);
    return 0;
}

/**
 * @brief http请求响应状态回调，这个回调没有用，因为http请求不带状态
 */
static auto OnRequestStatusCb(http_parser *p, const char *buf, size_t len)
    -> int {
    LOG_DEBUG(sys_logger, "on_request_status_cb, should not happen");
    return 0;
}

/**
 * @brief http消息体回调
 * @note
 * 当传输编码是chunked时，每个chunked数据段都会触发一次当前回调，所以用append的方法将所有数据组合到一起
 */
static auto OnRequestBodyCb(http_parser *p, const char *buf, size_t len)
    -> int {
    std::string body(buf, len);
    LOG_CUSTOM_DEBUG(sys_logger, "on_request_body_cb, body is: %s",
                     body.c_str())
    auto *parser = static_cast<HttpRequestParser *>(p->data);
    parser->GetData()->AppendBody(body);
    return 0;
}

static http_parser_settings s_request_settings = {
    .on_message_begin = OnRequestMessageBeginCb,
    .on_url = OnRequestUrlCb,
    .on_status = OnRequestStatusCb,
    .on_header_field = OnRequestHeaderFieldCb,
    .on_header_value = OnRequestHeaderValueCb,
    .on_headers_complete = OnRequestHeadersCompleteCb,
    .on_body = OnRequestBodyCb,
    .on_message_complete = OnRequestMessageCompleteCb,
    .on_chunk_header = OnRequestChunkHeaderCb,
    .on_chunk_complete = OnRequestChunkCompleteCb};

HttpRequestParser::HttpRequestParser() {
    http_parser_init(&m_parser, HTTP_REQUEST);
    m_data.reset(new HttpRequest);
    m_parser.data = this;
    m_error = 0;
    m_finished = false;
}

auto HttpRequestParser::Execute(char *data, size_t len) -> size_t {
    size_t nparsed =
        http_parser_execute(&m_parser, &s_request_settings, data, len);
    if (m_parser.upgrade != 0) {
        // "Upgrade" 是一个请求头，用于指示客户端希望升级到另一个协议
        LOG_DEBUG(sys_logger, "found upgrade, ignore");
        SetError(HPE_UNKNOWN);
    } else if (m_parser.http_errno != 0) {
        LOG_CUSTOM_DEBUG(sys_logger, "parse request fail: %s",
                         http_errno_name(HTTP_PARSER_ERRNO(&m_parser)))
        SetError((int8_t)m_parser.http_errno);
    } else {
        if (nparsed < len) {
            memmove(data, data + nparsed, (len - nparsed));
        }
    }
    return nparsed;
}

/**
 * @brief http响应开始解析回调函数
 */
static auto OnResponseMessageBeginCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_response_message_begin_cb");
    return 0;
}

/**
 * @brief http响应头部字段解析结束，可获取头部信息字段，如status_code/version等
 * @note
 * 返回0表示成功，返回1表示该HTTP消息无消息体，返回2表示无消息体并且该连接后续不会再有消息
 */
static auto OnResponseHeadersCompleteCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_response_headers_complete_cb");
    auto *parser = static_cast<HttpResponseParser *>(p->data);
    parser->GetData()->SetVersion(((p->http_major) << 0x4) | (p->http_minor));
    parser->GetData()->SetStatus(static_cast<HttpStatus>(p->status_code));
    return 0;
}

/**
 * @brief http响应解析结束回调
 */
static auto OnResponseMessageCompleteCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_response_message_complete_cb");
    auto *parser = static_cast<HttpResponseParser *>(p->data);
    parser->SetFinished(true);
    return 0;
}

/**
 * @brief http分段头部回调，可获取分段长度
 */
static auto OnResponseChunkHeaderCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_response_chunk_header_cb");
    return 0;
}

/**
 * @brief http分段结束回调，表示全部分段已解析完成
 */
static auto OnResponseChunkCompleteCb(http_parser *p) -> int {
    LOG_DEBUG(sys_logger, "on_response_chunk_complete_cb");
    return 0;
}

/**
 * @brief http响应url解析完成回调，这个回调没有意义，因为响应不会携带url
 */
static auto OnResponseUrlCb(http_parser *p, const char *buf, size_t len)
    -> int {
    LOG_DEBUG(sys_logger, "on_response_url_cb, should not happen");
    return 0;
}

/**
 * @brief http响应首部字段名称解析完成回调
 */
static auto OnResponseHeaderFieldCb(http_parser *p, const char *buf, size_t len)
    -> int {
    std::string field(buf, len);
    LOG_CUSTOM_DEBUG(sys_logger, "on_response_header_field_cb, field is: %s",
                     field.c_str())
    auto *parser = static_cast<HttpResponseParser *>(p->data);
    parser->SetField(field);
    return 0;
}

/**
 * @brief http响应首部字段值解析完成回调
 */
static auto OnResponseHeaderValueCb(http_parser *p, const char *buf, size_t len)
    -> int {
    std::string value(buf, len);
    LOG_CUSTOM_DEBUG(sys_logger, "on_response_header_value_cb, value is: %s",
                     value.c_str())
    auto *parser = static_cast<HttpResponseParser *>(p->data);
    parser->GetData()->SetHeader(parser->GetField(), value);
    return 0;
}

/**
 * @brief http响应状态回调
 */
static auto OnResponseStatusCb(http_parser *p, const char *buf, size_t len)
    -> int {
    LOG_CUSTOM_DEBUG(
        sys_logger,
        "on_response_status_cb, status code is: %u, status msg is: %s",
        p->status_code, std::string(buf, len).c_str())
    auto *parser = static_cast<HttpResponseParser *>(p->data);
    parser->GetData()->SetStatus(static_cast<HttpStatus>(p->status_code));
    return 0;
}

/**
 * @brief http响应消息体回调
 */
static auto OnResponseBodyCb(http_parser *p, const char *buf, size_t len)
    -> int {
    std::string body(buf, len);
    LOG_CUSTOM_DEBUG(sys_logger, "on_response_body_cb, body is: %s",
                     body.c_str())
    auto *parser = static_cast<HttpResponseParser *>(p->data);
    parser->GetData()->AppendBody(body);
    return 0;
}

static http_parser_settings s_response_settings = {
    .on_message_begin = OnResponseMessageBeginCb,
    .on_url = OnResponseUrlCb,
    .on_status = OnResponseStatusCb,
    .on_header_field = OnResponseHeaderFieldCb,
    .on_header_value = OnResponseHeaderValueCb,
    .on_headers_complete = OnResponseHeadersCompleteCb,
    .on_body = OnResponseBodyCb,
    .on_message_complete = OnResponseMessageCompleteCb,
    .on_chunk_header = OnResponseChunkHeaderCb,
    .on_chunk_complete = OnResponseChunkCompleteCb};

HttpResponseParser::HttpResponseParser() {
    http_parser_init(&m_parser, HTTP_RESPONSE);
    m_data.reset(new HttpResponse);
    m_parser.data = this;
    m_error = 0;
    m_finished = false;
}

auto HttpResponseParser::Execute(char *data, size_t len) -> size_t {
    size_t nparsed =
        http_parser_execute(&m_parser, &s_response_settings, data, len);
    if (m_parser.http_errno != 0) {
        LOG_CUSTOM_DEBUG(sys_logger, "parse response fail: %s",
                         http_errno_name(HTTP_PARSER_ERRNO(&m_parser)))
        SetError((int8_t)m_parser.http_errno);
    } else {
        if (nparsed < len) {
            memmove(data, data + nparsed, (len - nparsed));
        }
    }
    return nparsed;
}

}  // namespace wtsclwq