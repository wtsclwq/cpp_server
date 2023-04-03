//
// Created by wtsclwq on 23-4-2.
//

#pragma once
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../util/http_util.h"
#include "http_status.h"
namespace wtsclwq {

class HttpResponse {
  public:
    using ptr = std::shared_ptr<HttpResponse>;
    using MappingType = std::map<std::string, std::string, CaseInsensitiveLess>;

    HttpResponse(const HttpResponse& other) = delete;
    HttpResponse(HttpResponse&& other) = delete;
    auto operator=(const HttpResponse& other) = delete;
    auto operator=(HttpResponse&& other) = delete;
    ~HttpResponse() = default;

    HttpResponse();

    HttpResponse(uint8_t version, bool close);

    /**
     * @brief 获取头部参数的字符串，携带默认值、
     * @param[in] key key
     * @param[in] def 默认值
     * @return 成功返回相应的参数字符串，失败则返回给定默认值
     */
    auto GetHeaderStrWithDef(const std::string& key,
                             const std::string& def = "") const -> std::string;

    /**
     * @brief 设置头部参数
     */
    void SetHeader(const std::string& key, const std::string& val);

    /**
     * @brief 删除头部参数
     */
    void DelHeader(const std::string& key);

    void SetRedirect(const std::string& uri);

    /**
     * @brief 设置Cookie信息
     * @param key
     * @param val
     * @param expired
     * @param path
     * @param domain
     * @param secure
     */
    void SetCookie(const std::string& key, const std::string& val,
                   time_t expired = 0, const std::string& path = "",
                   const std::string& domain = "", bool secure = false);
    /**
     * @brief 检查并获取头部参数，同时转换类型，携带默认值
     * @tparam T 转换类型
     * @param[in] key
     * @param[out] val 出参数，获取达到T类型的参数
     * @param[out] def 默认值
     * @return 成功返回true并利用val传出，否则返回null且传出默认值
     */
    template <typename T>
    auto CheckAndGetHeaderWithDefAs(const std::string& key, T& val,
                                    const T& def = T()) -> bool;

    /**
     * @brief 获取头部参数，同时转换类型，携带默认值
     * @tparam T 转换类型
     * @param[in] key
     * @param[in] def 默认值
     * @return 成功则返回转换后的参数值，失败则返回默认值
     */
    template <typename T>
    auto GetHeaderWithDefAs(const std::string& key, const T& def = T()) -> T;

    /**
     * @brief 将可读信息倒入stream
     * @param os 要倒入的stream
     * @return 倒入后的stream
     */
    auto Dump(std::ostream& os) -> std::ostream&;

    /**
     * @brief 返回可读字符串
     */
    auto ToString() -> std::string;

    auto GetStatus() const -> HttpStatus;

    void SetStatus(HttpStatus m_status);

    auto GetVersion() const -> uint8_t;

    void SetVersion(uint8_t m_version);

    auto IsAutoClose() const -> bool;

    void SetIsAutoClose(bool m_is_auto_close);

    auto IsWebSocket() const -> bool;

    void SetIsWebSocket(bool m_is_web_socket);

    auto GetBody() const -> const std::string&;

    void SetBody(const std::string& m_body);

    void AppendBody(const std::string& value);

    auto GetReason() const -> const std::string&;

    void SetReason(const std::string& m_reason);

    auto GetHeaders() const -> const MappingType&;

    void SetHeaders(const MappingType& m_headers);

    auto GetCookies() const -> const std::vector<std::string>&;

    void SetCookies(const std::vector<std::string>& m_cookies);

  private:
    HttpStatus m_status;                 // 相应状态
    uint8_t m_version;                   // 协议版本
    bool m_is_auto_close;                // 是否自动关闭
    bool m_is_web_socket;                // 是否是websocket
    std::string m_body;                  // 消息体
    std::string m_reason;                // 响应原因
    MappingType m_headers;               // 头部参数表
    std::vector<std::string> m_cookies;  // cookies列表
};
template <typename T>
auto HttpResponse::CheckAndGetHeaderWithDefAs(const std::string& key, T& val,
                                              const T& def) -> bool {}

template <typename T>
auto HttpResponse::GetHeaderWithDefAs(const std::string& key, const T& def)
    -> T {}

auto operator<<(std::ostream& os, const HttpResponse& response)
    -> std::ostream&;
}  // namespace wtsclwq
