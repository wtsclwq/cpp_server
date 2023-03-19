/*
 * @Description:
 * @author: wtsclwq
 * @Date: 2023-03-17 23:19:16
 * @LastEditTime: 2023-03-19 16:46:34
 */
#pragma once

#include <cassert>
#include <string>

#include "thread_util.h"

const int TRACE_SIZE = 100;
const int SKIP_SIZE = 2;
#define WTSCLWQ_ASSERT(x, w) /* NOLINT */                       \
    if (!(x)) {              /* NOLINT */                       \
        std::stringstream sstream;                              \
        sstream << "ASSERTION: " << #x                          \
                << ", MESSAGE: " << static_cast<const char*>(w) \
                << "\nbacktrace:\n"                             \
                << wtsclwq::BacktraceToString(100, 2, "    ");  \
        LOG_ERROR(ROOT_LOGGER, sstream.str());                  \
        assert((x)); /* NOLINT */                               \
    }
