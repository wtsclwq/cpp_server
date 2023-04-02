//
// Created by wtsclwq on 23-4-2.
//

#include "../include/util/string_util.h"

#include <cstring>

namespace wtsclwq {
auto StringUtil::Format(const char* fmt, ...) -> std::string {
    va_list ap;
    va_start(ap, fmt);
    auto v = Formatv(fmt, ap);
    va_end(ap);
    return v;
}

auto StringUtil::Formatv(const char* fmt, va_list ap) -> std::string {
    char* buf = nullptr;
    auto len = vasprintf(&buf, fmt, ap);
    if (len == -1) {
        return "";
    }
    std::string ret(buf, len);
    free(buf);
    return ret;
}

// 用来快速识别一个字符是否是合法的uri字符
static const char uri_chars[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,

    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

// 用来快速映射16进制字符‘1’，‘2’....'a'...'f'的十进制
static const char xdigit_chars[256] = {
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  1,  2,  3, 4, 5, 6, 7, 8, 9,  0,  0,
    0,  0,  0,  0, 0, 10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 10, 11, 12,
    13, 14, 15, 0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0,  0,  0,
    0,  0,  0,  0, 0, 0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0,
};

#define CHAR_IS_UNRESERVED(c) (uri_chars[(unsigned char)(c)])

//-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
auto StringUtil::UrlEncode(const std::string& str, bool space_as_plus)
    -> std::string {
    static const char* hexdigits = "0123456789ABCDEF";
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c) {
        if (!CHAR_IS_UNRESERVED(*c)) {
            if (ss == nullptr) {
                ss = new std::string;
                ss->reserve(str.size() * 1.2);
                ss->append(str.c_str(), c - str.c_str());
            }
            if (*c == ' ' && space_as_plus) {
                ss->append(1, '+');
            } else {
                ss->append(1, '%');
                ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                ss->append(1, hexdigits[*c & 0xf]);
            }
        } else if (ss != nullptr) {
            ss->append(1, *c);
        }
    }
    if (ss == nullptr) {
        return str;
    }
    std::string rt = *ss;
    delete ss;
    return rt;
}

auto StringUtil::UrlDecode(const std::string& str, bool space_as_plus)
    -> std::string {
    std::string* ss = nullptr;
    const char* end = str.c_str() + str.length();
    for (const char* c = str.c_str(); c < end; ++c) {
        if (*c == '+' && space_as_plus) {
            if (ss == nullptr) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, ' ');
        } else if (*c == '%' && (c + 2) < end && (isxdigit(*(c + 1)) != 0) &&
                   (isxdigit(*(c + 2)) != 0)) {
            if (ss == nullptr) {
                ss = new std::string;
                ss->append(str.c_str(), c - str.c_str());
            }
            ss->append(1, static_cast<char>(
                              xdigit_chars[static_cast<int>(*(c + 1))] << 4 |
                              xdigit_chars[static_cast<int>(*(c + 2))]));
            c += 2;
        } else if (ss != nullptr) {
            ss->append(1, *c);
        }
    }
    if (ss == nullptr) {
        return str;
    }
    std::string rt = *ss;
    delete ss;
    return rt;
}

auto StringUtil::Trim(const std::string& str, const std::string& delimit)
    -> std::string {
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos) {
        return "";
    }
    auto end = str.find_last_not_of(delimit);
    return str.substr(begin, end - begin + 1);
}

auto StringUtil::TrimLeft(const std::string& str, const std::string& delimit)
    -> std::string {
    auto begin = str.find_first_not_of(delimit);
    if (begin == std::string::npos) {
        return "";
    }
    return str.substr(begin);
}

auto StringUtil::TrimRight(const std::string& str, const std::string& delimit)
    -> std::string {
    auto end = str.find_last_not_of(delimit);
    if (end == std::string::npos) {
        return "";
    }
    return str.substr(0, end);
}

auto StringUtil::WStringToString(const std::wstring& ws) -> std::string {
    std::string str_locale = setlocale(LC_ALL, "");
    const wchar_t* wch_src = ws.c_str();
    size_t n_dest_size = wcstombs(nullptr, wch_src, 0) + 1;
    char* ch_dest = new char[n_dest_size];
    memset(ch_dest, 0, n_dest_size);
    wcstombs(ch_dest, wch_src, n_dest_size);
    std::string str_result = ch_dest;
    delete[] ch_dest;
    setlocale(LC_ALL, str_locale.c_str());
    return str_result;
}

auto StringUtil::StringToWString(const std::string& s) -> std::wstring {
    std::string str_locale = setlocale(LC_ALL, "");
    const char* chSrc = s.c_str();
    size_t n_dest_size = mbstowcs(nullptr, chSrc, 0) + 1;
    auto* wch_dest = new wchar_t[n_dest_size];
    wmemset(wch_dest, 0, n_dest_size);
    mbstowcs(wch_dest, chSrc, n_dest_size);
    std::wstring wstr_result = wch_dest;
    delete[] wch_dest;
    setlocale(LC_ALL, str_locale.c_str());
    return wstr_result;
}

auto StringUtil::TimeToStr(time_t ts, const std::string& format)
    -> std::string {
    struct tm tm;
    localtime_r(&ts, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), format.c_str(), &tm);
    return buf;
}

auto StringUtil::StrToTime(const char* str, const char* format) -> time_t {
    struct tm t;
    memset(&t, 0, sizeof(t));
    if (strptime(str, format, &t) == nullptr) {
        return 0;
    }
    return mktime(&t);
}

}  // namespace wtsclwq