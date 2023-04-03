/**
 * @file http_parser.h
 * @brief HTTP协议解析封装
 * @version 0.1
 * @date 2021-09-25
 */
#pragma once

#include "http_request.h"
#include "http_response.h"
#include "parser.h"

namespace wtsclwq {

/**
 * @brief HTTP请求解析类
 */
class HttpRequestParser {
  public:
    /// HTTP解析类的智能指针
    using ptr = std::shared_ptr<HttpRequestParser>;

    /**
     * @brief 构造函数
     */
    HttpRequestParser();

    /**
     * @brief 解析协议
     * @param[in, out] data 协议文本内存
     * @param[in] len 协议文本内存长度
     * @return 返回实际解析的长度,并且将已解析的数据移除
     */
    auto Execute(char *data, size_t len) -> size_t;

    /**
     * @brief 是否解析完成
     * @return 是否解析完成
     */
    auto IsFinished() const -> bool { return m_finished; }

    /**
     * @brief 设置是否解析完成
     */
    void SetFinished(bool v) { m_finished = v; }

    /**
     * @brief 是否有错误
     * @return 是否有错误
     */
    auto HasError() const -> bool { return m_error != 0; }

    /**
     * @brief 设置错误
     * @param[in] v 错误值
     */
    void SetError(int v) { m_error = v; }

    /**
     * @brief 返回HttpRequest结构体
     */
    auto GetData() const -> HttpRequest::ptr { return m_data; }

    /**
     * @brief 获取http_parser结构体
     */
    auto GetParser() const -> const http_parser & { return m_parser; }

    /**
     * @brief 获取当前的HTTP头部field
     */
    auto GetField() const -> const std::string & { return m_field; }

    /**
     * @brief 设置当前的HTTP头部field
     */
    void SetField(const std::string &v) { m_field = v; }

  public:
    /**
     * @brief 返回HttpRequest协议解析的缓存大小
     */
    static auto GetHttpRequestBufferSize() -> uint64_t;

    /**
     * @brief 返回HttpRequest协议的最大消息体大小
     */
    static auto GetHttpRequestMaxBodySize() -> uint64_t;

  private:
    /// http_parser
    http_parser m_parser;
    /// HttpRequest
    HttpRequest::ptr m_data;
    /// 错误码，参考http_errno
    int m_error;
    /// 是否解析结束
    bool m_finished;
    /// 当前的HTTP头部field，http-parser解析HTTP头部是field和value分两次返回
    std::string m_field;
};

/**
 * @brief Http响应解析结构体
 */
class HttpResponseParser {
  public:
    /// 智能指针类型
    using ptr = std::shared_ptr<HttpResponseParser>;

    /**
     * @brief 构造函数
     */
    HttpResponseParser();

    /**
     * @brief 解析HTTP响应协议
     * @param[in, out] data 协议数据内存
     * @param[in] len 协议数据内存大小
     * @return 返回实际解析的长度,并且移除已解析的数据
     */
    auto Execute(char *data, size_t len) -> size_t;

    /**
     * @brief 是否解析完成
     */
    auto IsFinished() const -> bool { return m_finished; }

    /**
     * @brief 设置是否解析完成
     */
    void SetFinished(bool v) { m_finished = v; }

    /**
     * @brief 是否有错误
     */
    auto HasError() const -> bool { return m_error != 0; }

    /**
     * @brief 设置错误码
     * @param[in] v 错误码
     */
    void SetError(int v) { m_error = v; }

    /**
     * @brief 返回HttpResponse
     */
    auto GetData() const -> HttpResponse::ptr { return m_data; }

    /**
     * @brief 返回http_parser
     */
    auto GetParser() const -> const http_parser & { return m_parser; }

    /**
     * @brief 获取当前的HTTP头部field
     */
    auto GetField() const -> const std::string & { return m_field; }

    /**
     * @brief 设置当前的HTTP头部field
     */
    void SetField(const std::string &v) { m_field = v; }

  public:
    /**
     * @brief 返回HTTP响应解析缓存大小
     */
    static auto GetHttpResponseBufferSize() -> uint64_t;

    /**
     * @brief 返回HTTP响应最大消息体大小
     */
    static auto GetHttpResponseMaxBodySize() -> uint64_t;

  private:
    /// HTTP响应解析器
    http_parser m_parser;
    /// HTTP响应对象
    HttpResponse::ptr m_data;
    /// 错误码
    int m_error;
    /// 是否解析结束
    bool m_finished;
    /// 当前的HTTP头部field
    std::string m_field;
};

}  // namespace wtsclwq
