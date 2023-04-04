//
// Created by wtsclwq on 23-4-4.
//

#pragma once
#include "../concurrency/lock.h"
#include "../serialize/stream.h"
#include "socket.h"

namespace wtsclwq {

class SocketStream : public Stream {
  public:
    using ptr = std::shared_ptr<SocketStream>;

    SocketStream(const SocketStream& other) = delete;
    SocketStream(SocketStream&& other) = delete;
    auto operator=(const SocketStream& other) = delete;
    auto operator=(SocketStream&& other) = delete;
    SocketStream() = delete;

    explicit SocketStream(Socket::ptr socket, bool owner = true);

    ~SocketStream() override;

    /**
     * @brief 从流中读取数据到buffer
     * @param[in] buffer 接受数据的buffer
     * @param[in] length 接受地的大小
     * @return
     * @retval > 0 实际读取的数据大小
     * @retval = 0 被关闭
     * @retval < 0 出错
     */
    auto ReadToBuf(void* buffer, size_t length) -> int override;

    /**\
     * @brief 从流中读取数据到ByteArray
     * @param byte_array 接受数据的ByteArray
     * @param length 接受地的大小
     * @return
     * @retval > 0 实际读取的数据大小
     * @retval = 0 被关闭
     * @retval < 0 出错
     */
    auto ReadToArray(ByteArray::ptr byte_array, size_t length) -> int override;

    /**
     * @brief 向流写buffer中的数据
     * @param[in] buffer 写数据的内存
     * @param[in] length 写入数据的内存大小
     * @return
     *      @retval >0 返回写入到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */

    auto WriteFromBuf(const void* buffer, size_t length) -> int override;

    /**
     * @brief 向流写ByteArray中的数据
     * @param[in] byte_array 写数据的ByteArray
     * @param[in] length 写入数据的内存大小
     * @return
     *      @retval >0 返回写入到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    auto WriteFromArray(ByteArray::ptr byte_array, size_t length) -> int override;

    void Close() override;

    auto IsConnected() const -> bool;

  protected:
    Socket::ptr m_socket;  // Socket对象
    bool m_owner;          // 是否主控
};

}  // namespace wtsclwq
