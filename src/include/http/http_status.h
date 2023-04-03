//
// Created by wtsclwq on 23-4-2.
//

#pragma once
#include <string>

#include "parser.h"
namespace wtsclwq {

enum class HttpStatus {
#define DEF_STATUS(code, name, desc) name = (code),
    HTTP_STATUS_MAP(DEF_STATUS)
#undef DEF_STATUS
};

auto HttpStatusToString(const HttpStatus& state) -> std::string;

}  // namespace wtsclwq
