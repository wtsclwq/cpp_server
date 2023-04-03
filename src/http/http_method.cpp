//
// Created by wtsclwq on 23-4-2.
//

#include "../include/http/http_method.h"

#include <cstring>

namespace wtsclwq {
auto StringToHttpMethod(const std::string& str) -> HttpMethod {
#define GET_METHOD(num, name, string)        \
    if (strcmp(#string, str.c_str()) == 0) { \
        return HttpMethod::name;             \
    }
    HTTP_METHOD_MAP(GET_METHOD);
#undef GET_METHOD
    return HttpMethod::INVALID_METHOD;
}

auto CharsToHttpMethod(const char* str) -> HttpMethod {
#define GET_METHOD(num, name, string)                  \
    if (strncmp(#string, str, strlen(#string)) == 0) { \
        return HttpMethod::name;                       \
    }
    HTTP_METHOD_MAP(GET_METHOD);
#undef GET_METHOD
    return HttpMethod::INVALID_METHOD;
}

// 注意虽然每个string的长度可能不同，但是其实这是一个二维字符数组，因此每一行的长度相同
static const char* s_method_string[] = {
#define METHOD_STRING(num, name, string) #string,
    HTTP_METHOD_MAP(METHOD_STRING)
#undef XX
};

auto HttpMethodToString(const HttpMethod& method) -> const char* {
    uint32_t idx = static_cast<uint32_t>(method);
    if (idx >= (sizeof(s_method_string) / sizeof(s_method_string[0]))) {
        return "<unknown>";
    }
    return s_method_string[idx];
}
}  // namespace wtsclwq