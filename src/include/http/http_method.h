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

#include "parser.h"
namespace wtsclwq {

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
