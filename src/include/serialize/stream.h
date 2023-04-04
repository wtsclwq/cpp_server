//
// Created by wtsclwq on 23-4-4.
//

#pragma once
#include <memory>

#include "byte_array.h"
namespace wtsclwq {

class Stream {
  public:
    using ptr = std::shared_ptr<Stream>;

    virtual ~Stream() = default;

    /**
     * @brief 从流中读取数据到buffer
     * @param[in] buffer 接受数据的buffer
     * @param[in] length 接受地的大小
     * @return
     * @retval > 0 实际读取的数据大小
     * @retval = 0 被关闭
     * @retval < 0 出错
     */
    virtual auto ReadToBuf(void* buffer, size_t length) -> int = 0;

    /**\
     * @brief 从流中读取数据到ByteArray
     * @param byte_array 接受数据的ByteArray
     * @param length 接受地的大小
     * @return
     * @retval > 0 实际读取的数据大小
     * @retval = 0 被关闭
     * @retval < 0 出错
     */
    virtual auto ReadToArray(ByteArray::ptr byte_array, size_t length)
        -> int = 0;

    /**
     * @brief 从流中读取固定长度的数据到buffer
     * @param[out] buffer 接收数据的内存
     * @param[in] length 接收数据的内存大小
     * @return
     *      @retval >0 返回接收到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    virtual auto ReadFixSizeToBuf(void* buffer, size_t length) -> int;

    /**
     * @brief 从流中读取固定长度的数据到ByteArray
     * @param[out] ba 接收数据的ByteArray
     * @param[in] length 接收数据的内存大小
     * @return
     *      @retval >0 返回接收到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    virtual auto ReadFixSizeToArray(ByteArray::ptr ba, size_t length) -> int;

    /**
     * @brief 向流写buffer中的数据
     * @param[in] buffer 写数据的内存
     * @param[in] length 写入数据的内存大小
     * @return
     *      @retval >0 返回写入到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    virtual auto WriteFromBuf(const void* buffer, size_t length) -> int = 0;

    /**
     * @brief 向流写ByteArray中的数据
     * @param[in] byte_array 写数据的ByteArray
     * @param[in] length 写入数据的内存大小
     * @return
     *      @retval >0 返回写入到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    virtual auto WriteFromArray(ByteArray::ptr byte_array, size_t length) -> int = 0;

    /**
     * @brief 向流写固定长度的buffer中的数据
     * @param[in] buffer 写数据的内存
     * @param[in] length 写入数据的内存大小
     * @return
     *      @retval >0 返回写入到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    virtual auto WriteFixSizeFromBuf(const void* buffer, size_t length) -> int;

    /**
     * @brief 向流写固定长度的ByteArray中的数据
     * @param[in] ba 写数据的ByteArray
     * @param[in] length 写入数据的内存大小
     * @return
     *      @retval >0 返回写入到的数据的实际大小
     *      @retval =0 被关闭
     *      @retval <0 出现流错误
     */
    virtual auto WriteFixSizeFromArray(ByteArray::ptr ba, size_t length) -> int;

    /**
     * @brief 关闭流
     */
    virtual void Close() = 0;
};

}  // namespace wtsclwq
