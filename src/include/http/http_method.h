//
// Created by wtsclwq on 23-4-2.
//

#pragma once

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
namespace wtsclwq {
#define HTTP_METHOD_MAP(ITEM)          \
    ITEM(0, DELETE, DELETE)            \
    ITEM(1, GET, GET)                  \
    ITEM(2, HEAD, HEAD)                \
    ITEM(3, POST, POST)                \
    ITEM(4, PUT, PUT)                  \
    /* pathological */                 \
    ITEM(5, CONNECT, CONNECT)          \
    ITEM(6, OPTIONS, OPTIONS)          \
    ITEM(7, TRACE, TRACE)              \
    /* WebDAV */                       \
    ITEM(8, COPY, COPY)                \
    ITEM(9, LOCK, LOCK)                \
    ITEM(10, MKCOL, MKCOL)             \
    ITEM(11, MOVE, MOVE)               \
    ITEM(12, PROPFIND, PROPFIND)       \
    ITEM(13, PROPPATCH, PROPPATCH)     \
    ITEM(14, SEARCH, SEARCH)           \
    ITEM(15, UNLOCK, UNLOCK)           \
    ITEM(16, BIND, BIND)               \
    ITEM(17, REBIND, REBIND)           \
    ITEM(18, UNBIND, UNBIND)           \
    ITEM(19, ACL, ACL)                 \
    /* subversion */                   \
    ITEM(20, REPORT, REPORT)           \
    ITEM(21, MKACTIVITY, MKACTIVITY)   \
    ITEM(22, CHECKOUT, CHECKOUT)       \
    ITEM(23, MERGE, MERGE)             \
    /* upnp */                         \
    ITEM(24, MSEARCH, MSEARCH)         \
    ITEM(25, NOTIFY, NOTIFY)           \
    ITEM(26, SUBSCRIBE, SUBSCRIBE)     \
    ITEM(27, UNSUBSCRIBE, UNSUBSCRIBE) \
    /* RFC-5789 */                     \
    ITEM(28, PATCH, PATCH)             \
    ITEM(29, PURGE, PURGE)             \
    /* CalDAV */                       \
    ITEM(30, MKCALENDAR, MKCALENDAR)   \
    /* RFC-2068, section 19.6.1.2 */   \
    ITEM(31, LINK, LINK)               \
    ITEM(32, UNLINK, UNLINK)           \
    /* icecast */                      \
    ITEM(33, SOURCE, SOURCE)

enum class HttpMethod {
#define DEF_METHOD(code, name, desc) name = ((code)),
    HTTP_METHOD_MAP(DEF_METHOD)
#undef DEF_METHOD
        INVALID_METHOD
};

/**
 * @brief 将string转换为HTTP_METHOD
 */
auto StringToHttpMethod(const std::string &str) -> HttpMethod;

/**
 * @brief char* 转换为HTTP_METHOD
 */
auto CharsToHttpMethod(const char *str) -> HttpMethod;

/**
 * @brief 将HTTP_METHOD转换为字符串
 */
auto HttpMethodToString(const HttpMethod &method) -> const char *;

}  // namespace wtsclwq
