//
// Created by wtsclwq on 23-4-2.
//

#pragma once
#include <map>
#include <memory>
#include <string>

#include "../util/http_util.h"
#include "http_method.h"
#include "http_response.h"
namespace wtsclwq {

class HttpRequest {
  public:
    using MappingType = std::map<std::string, std::string, CaseInsensitiveLess>;
    using ptr = std::shared_ptr<HttpRequest>;

    HttpRequest(const HttpRequest& other) = delete;
    HttpRequest(HttpRequest&& other) = delete;
    auto operator=(const HttpRequest& other) = delete;
    auto operator=(HttpRequest&& other) = delete;
    ~HttpRequest() = default;

    HttpRequest();
    HttpRequest(uint8_t version, bool close);

    /**
     * @brief 初始化请求
     */
    void Init();

    /**
     * @brief 初始化request所有参数
     */
    void InitParam();

    /**
     * @brief 初始化Get请求地址参数
     */
    void InitQueryParam();

    /**
     * @brief 初始化请求体参数
     */
    void InitBodyParam();

    /**
     * @brief 初始化cookie
     */
    void InitCookies();

    /**
     * @brief 获取头部参数的字符串，并携带默认值
     * @param[in] key key
     * @param[in] def 默认值
     * @return 存在则返回参数，失败则返回默认值
     */
    auto GetHeaderStrWithDef(const std::string& key,
                             const std::string& def = "") -> std::string;

    /**
     * @brief 获取请求参数的字符串，并携带默认值
     * @param[in] key key
     * @param[in] def 默认值
     * @return 存在则返回参数，失败则返回默认值
     */
    auto GetParamStrWithDef(const std::string& key, const std::string& def = "")
        -> std::string;

    /**
     * @brief 获取Cookie的字符串，并携带默认值
     * @param[in] key key
     * @param[in] def 默认值
     * @return 存在则返回参数，失败则返回默认值
     */
    auto GetCookieStrWithDef(const std::string& key,
                             const std::string& def = "") -> std::string;

    /**
     * @brief 设置头部参数
     */
    void SetHeader(const std::string& key, const std::string& val);

    /**
     * @brief 设置请求参数
     */
    void SetParam(const std::string& key, const std::string& val);

    /**
     * @brief 设置头部参数
     */
    void SetCookie(const std::string& key, const std::string& val);

    /**
     * @brief 删除头部参数
     */
    void DelHeader(const std::string& key);

    /**
     * @brief 删除请求参数
     */
    void DelParam(const std::string& key);

    /**
     * @brief 删除Cookie参数
     */
    void DelCookie(const std::string& key);

    /**
     * @brief 判断头部是否存在该参数
     */
    auto HashHeader(const std::string& key) -> bool;

    /**
     * @brief 判断请求是否存在该参数
     */
    auto HashParam(const std::string& key) -> bool;

    /**
     * @brief 判断Cookie是否存在该参数
     */
    auto HashCookie(const std::string& key) -> bool;

    /**
     * @brief 创建响应
     * @return
     */
    auto CreateRespense() -> std::shared_ptr<HttpResponse>;

    /**
     * @brief 检查并获取头部中key对应的某一参数，并提供默认值
     * @tparam T 转换类型
     * @param[in] key key
     * @param[out] val 返回值
     * @param[in] def 默认值
     * @return 成功与否，失败时val = def
     */
    template <typename T>
    auto CheckAndGetHeaderWithDefAs(const std::string& key, T& val,
                                    const T& def = T()) -> bool;

    /**
     * @brief 获取key对应得到头部参数
     * @tparam T 转换类型
     * @param key key
     * @param def 默认值
     * @return 成功则返回参数指，失败则返回默认值def
     */
    template <typename T>
    auto GetHeaderWithDefAs(const std::string& key, const T& def = T()) -> T;

    /**
     * @brief 检查并获取HTTP请求的参数，并携带默认值
     * @tparam T 转换类型
     * @param[in] key
     * @param[out] val 返回值
     * @param[in] def 默认值
     * @return 成功返回true,失败返回false，并且val = def
     */
    template <typename T>
    auto CheckAndGetParamWithDefAs(const std::string& key, T& val,
                                   const T& def = T()) -> bool;

    /**
     * @brief 获取Http请求的参数，携带默认值
     * @tparam T 转换类型
     * @param[in] key
     * @param[in] def 默认值
     * @return 成功返回参数值，失败返默认值def
     */
    template <typename T>
    auto GetParamWithDefAs(const std::string& key, const T& def = T());

    /**
     * @brief 检查并获取HTTP请求的Cookie，并携带默认值
     * @tparam T 转换类型
     * @param[in] key
     * @param[out] val 返回值
     * @param[in] def 默认值
     * @return 成功返回true,失败返回false，并且val = def
     */
    template <typename T>
    auto CheckAndGetCookieWithDefAs(const std::string& key, T& val,
                                    const T& def = T()) -> bool;
    /**
     * @brief 获取Http请求的Cookie，携带默认值
     * @tparam T 转换类型
     * @param[in] key
     * @param[in] def 默认值
     * @return 成功返回参数值，失败返默认值def
     */
    template <typename T>
    auto GetCookieWithDefAs(const std::string& key, const T& def = T());

    /**
     * @brief 将可读信息导入stream
     * @param os 要导入stream
     * @return 导入后的stream
     */
    auto Dump(std::ostream& os) const -> std::ostream&;

    /**
     * @brief 返回可读字符串
     */
    auto ToString() const -> std::string;

    auto GetMethod() const -> HttpMethod;

    void SetMethod(HttpMethod method);

    auto GetVersion() const -> uint8_t;

    void SetVersion(uint8_t version);

    auto IsAutoClose() const -> bool;

    void SetIsAutoClose(bool is_auto_close);

    auto IsWebsocket() const -> bool;

    void SetIsWebsocket(bool is_websocket);

    auto GetPath() const -> const std::string&;

    void SetPath(const std::string& path);

    auto GetQuery() const -> const std::string&;

    void SetQuery(const std::string& query);

    auto GetFragment() const -> const std::string&;

    void SetFragment(const std::string& fragment);

    auto GetBody() const -> const std::string&;

    void SetBody(const std::string& body);

    auto GetHeaders() const -> const MappingType&;

    void SetHeaders(const MappingType& m_headers);

    auto GetParams() const -> const MappingType&;

    void SetParams(const MappingType& m_params);

    auto GetCookies() const -> const MappingType&;

    void SetCookies(const MappingType& m_cookies);

  private:
    HttpMethod m_method;          // 请求使用的方法
    uint8_t m_version;            // 请求使用的协议版本
    bool m_is_auto_close;         // 是否自动关闭
    bool m_is_websocket;          // 是否是websocket
    uint8_t m_parser_param_flag;  //
    std::string m_path;           // 请求路径
    std::string m_query;          // Get请求路径中的参数
    std::string m_fragment;       // 请求的fragment
    std::string m_body;           // 消息主体
    MappingType m_headers;        // 请求头参数表
    MappingType m_params;         // 请求参数表
    MappingType m_cookies;        // Cookie表
};

template <typename T>
auto HttpRequest::CheckAndGetHeaderWithDefAs(const std::string& key, T& val,
                                             const T& def) -> bool {
    return CheckGetWithDef(m_headers, key, val, def);
}

template <typename T>
auto HttpRequest::GetHeaderWithDefAs(const std::string& key, const T& def)
    -> T {
    return GetWithDef(m_headers, key, def);
}

template <typename T>
auto HttpRequest::CheckAndGetParamWithDefAs(const std::string& key, T& val,
                                            const T& def) -> bool {
    InitQueryParam();
    InitBodyParam();
    return CheckGetWithDef(m_params, key, val, def);
}

template <typename T>
auto HttpRequest::GetParamWithDefAs(const std::string& key, const T& def) {
    InitQueryParam();
    InitBodyParam();
    return GetWithDef(m_params, key, def);
}

template <typename T>
auto HttpRequest::CheckAndGetCookieWithDefAs(const std::string& key, T& val,
                                             const T& def) -> bool {
    InitCookies();
    return CheckGetWithDef(m_cookies, key, val, def);
}

template <typename T>
auto HttpRequest::GetCookieWithDefAs(const std::string& key, const T& def) {
    InitCookies();
    return GetWithDef(m_cookies, key, def);
}
auto operator<<(std::ostream& os, const HttpRequest& request) -> std::ostream&;
}  // namespace wtsclwq
