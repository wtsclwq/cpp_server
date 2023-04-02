//
// Created by wtsclwq on 23-4-2.
//

#pragma once
#include <string>

namespace wtsclwq {
#define HTTP_STATUS_MAP(ITEM)                                                 \
    ITEM(100, CONTINUE, Continue)                                             \
    ITEM(101, SWITCHING_PROTOCOLS, Switching Protocols)                       \
    ITEM(102, PROCESSING, Processing)                                         \
    ITEM(200, OK, OK)                                                         \
    ITEM(201, CREATED, Created)                                               \
    ITEM(202, ACCEPTED, Accepted)                                             \
    ITEM(203, NON_AUTHORITATIVE_INFORMATION, Non - Authoritative Information) \
    ITEM(204, NO_CONTENT, No Content)                                         \
    ITEM(205, RESET_CONTENT, Reset Content)                                   \
    ITEM(206, PARTIAL_CONTENT, Partial Content)                               \
    ITEM(207, MULTI_STATUS, Multi - Status)                                   \
    ITEM(208, ALREADY_REPORTED, Already Reported)                             \
    ITEM(226, IM_USED, IM Used)                                               \
    ITEM(300, MULTIPLE_CHOICES, Multiple Choices)                             \
    ITEM(301, MOVED_PERMANENTLY, Moved Permanently)                           \
    ITEM(302, FOUND, Found)                                                   \
    ITEM(303, SEE_OTHER, See Other)                                           \
    ITEM(304, NOT_MODIFIED, Not Modified)                                     \
    ITEM(305, USE_PROXY, Use Proxy)                                           \
    ITEM(307, TEMPORARY_REDIRECT, Temporary Redirect)                         \
    ITEM(308, PERMANENT_REDIRECT, Permanent Redirect)                         \
    ITEM(400, BAD_REQUEST, Bad Request)                                       \
    ITEM(401, UNAUTHORIZED, Unauthorized)                                     \
    ITEM(402, PAYMENT_REQUIRED, Payment Required)                             \
    ITEM(403, FORBIDDEN, Forbidden)                                           \
    ITEM(404, NOT_FOUND, Not Found)                                           \
    ITEM(405, METHOD_NOT_ALLOWED, Method Not Allowed)                         \
    ITEM(406, NOT_ACCEPTABLE, Not Acceptable)                                 \
    ITEM(407, PROXY_AUTHENTICATION_REQUIRED, Proxy Authentication Required)   \
    ITEM(408, REQUEST_TIMEOUT, Request Timeout)                               \
    ITEM(409, CONFLICT, Conflict)                                             \
    ITEM(410, GONE, Gone)                                                     \
    ITEM(411, LENGTH_REQUIRED, Length Required)                               \
    ITEM(412, PRECONDITION_FAILED, Precondition Failed)                       \
    ITEM(413, PAYLOAD_TOO_LARGE, Payload Too Large)                           \
    ITEM(414, URI_TOO_LONG, URI Too Long)                                     \
    ITEM(415, UNSUPPORTED_MEDIA_TYPE, Unsupported Media Type)                 \
    ITEM(416, RANGE_NOT_SATISFIABLE, Range Not Satisfiable)                   \
    ITEM(417, EXPECTATION_FAILED, Expectation Failed)                         \
    ITEM(421, MISDIRECTED_REQUEST, Misdirected Request)                       \
    ITEM(422, UNPROCESSABLE_ENTITY, Unprocessable Entity)                     \
    ITEM(423, LOCKED, Locked)                                                 \
    ITEM(424, FAILED_DEPENDENCY, Failed Dependency)                           \
    ITEM(426, UPGRADE_REQUIRED, Upgrade Required)                             \
    ITEM(428, PRECONDITION_REQUIRED, Precondition Required)                   \
    ITEM(429, TOO_MANY_REQUESTS, Too Many Requests)                           \
    ITEM(431, REQUEST_HEADER_FIELDS_TOO_LARGE,                                \
         Request Header Fields Too Large)                                     \
    ITEM(451, UNAVAILABLE_FOR_LEGAL_REASONS, Unavailable For Legal Reasons)   \
    ITEM(500, INTERNAL_SERVER_ERROR, Internal Server Error)                   \
    ITEM(501, NOT_IMPLEMENTED, Not Implemented)                               \
    ITEM(502, BAD_GATEWAY, Bad Gateway)                                       \
    ITEM(503, SERVICE_UNAVAILABLE, Service Unavailable)                       \
    ITEM(504, GATEWAY_TIMEOUT, Gateway Timeout)                               \
    ITEM(505, HTTP_VERSION_NOT_SUPPORTED, HTTP Version Not Supported)         \
    ITEM(506, VARIANT_ALSO_NEGOTIATES, Variant Also Negotiates)               \
    ITEM(507, INSUFFICIENT_STORAGE, Insufficient Storage)                     \
    ITEM(508, LOOP_DETECTED, Loop Detected)                                   \
    ITEM(510, NOT_EXTENDED, Not Extended)                                     \
    ITEM(511, NETWORK_AUTHENTICATION_REQUIRED, Network Authentication Required)

enum class HttpStatus {
#define DEF_STATUS(code, name, desc) name = (code),
    HTTP_STATUS_MAP(DEF_STATUS)
#undef DEF_STATUS
};

auto HttpStatusToString(const HttpStatus& state) -> std::string;

}  // namespace wtsclwq
