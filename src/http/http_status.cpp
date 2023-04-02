//
// Created by wtsclwq on 23-4-2.
//

#include "../include/http/http_status.h"

namespace wtsclwq {

auto HttpStatusToString(const HttpStatus& state) -> std::string {
    switch (state) {
#define GET_STR(code, name, msg) \
    case HttpStatus::name:       \
        return #msg;
        HTTP_STATUS_MAP(GET_STR);
#undef GET_STR
        default:
            return "<unknown>";
    }
}

}  // namespace wtsclwq