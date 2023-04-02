//
// Created by wtsclwq on 23-4-2.
//

#pragma once

#include <ctime>
#include <string>
namespace wtsclwq {

class StringUtil {
  public:
    static auto Format(const char* fmt, ...) -> std::string;

    static auto Formatv(const char* fmt, va_list ap) -> std::string;

    static auto UrlEncode(const std::string& str, bool space_as_plus = true)
        -> std::string;

    static auto UrlDecode(const std::string& str, bool space_as_plus = true)
        -> std::string;

    static auto Trim(const std::string& str,
                     const std::string& delimit = " \t\r\n") -> std::string;

    static auto TrimLeft(const std::string& str,
                         const std::string& delimit = " \t\r\n") -> std::string;

    static auto TrimRight(const std::string& str,
                          const std::string& delimit = " \t\r\n")
        -> std::string;

    static auto WStringToString(const std::wstring& ws) -> std::string;

    static auto StringToWString(const std::string& s) -> std::wstring;

    static auto TimeToStr(time_t ts = time(nullptr),
                          const std::string& format = "%Y-%m-%d %H:%M:%S")
        -> std::string;

    static auto StrToTime(const char* str,
                          const char* format = "%Y-%m-%d %H:%M:%S") -> time_t;
};

}  // namespace wtsclwq
