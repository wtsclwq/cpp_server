//
// Created by wtsclwq on 23-4-1.
//

#pragma once

#include <sys/socket.h>
#include <sys/types.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace wtsclwq {

class ByteArray {
  public:
    using ptr = std::shared_ptr<ByteArray>;

    struct Node {
        Node() = default;
        explicit Node(size_t size);
        ~Node();
        char* ptr{nullptr};   // 内存块地址指针
        Node* next{nullptr};  // 下一个内存块地址
        size_t size{0};       // 内存块大小
    };
    ByteArray(const ByteArray& other) = delete;
    ByteArray(ByteArray&& other) = delete;
    auto operator=(const ByteArray& other) -> ByteArray = delete;
    auto operator=(ByteArray&& other) -> ByteArray = delete;

    ByteArray();
    /**
     * @brief 用指定长度的内存块构造ByteArray
     * @param base_size 内存块大小
     */
    explicit ByteArray(size_t base_size);

    /**
     * @brief 析构函数
     */
    ~ByteArray();

    /**
     * @brief 写入固定长度int8_t类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedInt8(int8_t value);
    /**
     * @brief 写入固定长度uint8_t类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedUint8(uint8_t value);
    /**
     * @brief 写入固定长度int16_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedInt16(int16_t value);
    /**
     * @brief 写入固定长度uint16_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedUint16(uint16_t value);

    /**
     * @brief 写入固定长度int32_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedInt32(int32_t value);

    /**
     * @brief 写入固定长度uint32_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedUint32(uint32_t value);

    /**
     * @brief 写入固定长度int64_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedInt64(int64_t value);

    /**
     * @brief 写入固定长度uint64_t类型的数据(大端/小端)
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedUint64(uint64_t value);

    /**
     * @brief 写入float类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedFloat(float value);

    /**
     * @brief 写入double类型的数据
     * @post m_position += sizeof(value)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteFixedDouble(double value);

    /**
     * @brief 写入有符号Varint32类型的数据
     * @post m_position += 实际占用内存(1 ~ 5)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteInt32(int32_t value);
    /**
     * @brief 写入无符号Varint32类型的数据
     * @post m_position += 实际占用内存(1 ~ 5)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteUint32(uint32_t value);

    /**
     * @brief 写入有符号Varint64类型的数据
     * @post m_position += 实际占用内存(1 ~ 10)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteInt64(int64_t value);

    /**
     * @brief 写入无符号Varint64类型的数据
     * @post m_position += 实际占用内存(1 ~ 10)
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteUint64(uint64_t value);

    /**
     * @brief 写入std::string类型的数据,用uint16_t作为长度类型
     * @post m_position += 2 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteStringFixedUint16(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用uint32_t作为长度类型
     * @post m_position += 4 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteStringFixedUint32(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用uint64_t作为长度类型
     * @post m_position += 8 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteStringFixedUint64(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,用无符号Varint64作为长度类型
     * @post m_position += Varint64长度 + value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteStringVint(const std::string& value);

    /**
     * @brief 写入std::string类型的数据,无长度
     * @post m_position += value.size()
     *       如果m_position > m_size 则 m_size = m_position
     */
    void WriteStringWithoutLength(const std::string& value);

    /**
     * @brief 读取int8_t类型的数据
     * @pre getReadSize() >= sizeof(int8_t)
     * @post m_position += sizeof(int8_t);
     * @exception 如果getReadSize() < sizeof(int8_t) 抛出 std::out_of_range
     */
    auto ReadFixedInt8() -> int8_t;

    /**
     * @brief 读取uint8_t类型的数据
     * @pre getReadSize() >= sizeof(uint8_t)
     * @post m_position += sizeof(uint8_t);
     * @exception 如果getReadSize() < sizeof(uint8_t) 抛出 std::out_of_range
     */
    auto ReadFixedUint8() -> uint8_t;

    /**
     * @brief 读取int16_t类型的数据
     * @pre getReadSize() >= sizeof(int16_t)
     * @post m_position += sizeof(int16_t);
     * @exception 如果getReadSize() < sizeof(int16_t) 抛出 std::out_of_range
     */
    auto ReadFixedInt16() -> int16_t;

    /**
     * @brief 读取uint16_t类型的数据
     * @pre getReadSize() >= sizeof(uint16_t)
     * @post m_position += sizeof(uint16_t);
     * @exception 如果getReadSize() < sizeof(uint16_t) 抛出 std::out_of_range
     */
    auto ReadFixedUint16() -> uint16_t;

    /**
     * @brief 读取int32_t类型的数据
     * @pre getReadSize() >= sizeof(int32_t)
     * @post m_position += sizeof(int32_t);
     * @exception 如果getReadSize() < sizeof(int32_t) 抛出 std::out_of_range
     */
    auto ReadFixedInt32() -> int32_t;

    /**
     * @brief 读取uint32_t类型的数据
     * @pre getReadSize() >= sizeof(uint32_t)
     * @post m_position += sizeof(uint32_t);
     * @exception 如果getReadSize() < sizeof(uint32_t) 抛出 std::out_of_range
     */
    auto ReadFixedUint32() -> uint32_t;

    /**
     * @brief 读取int64_t类型的数据
     * @pre getReadSize() >= sizeof(int64_t)
     * @post m_position += sizeof(int64_t);
     * @exception 如果getReadSize() < sizeof(int64_t) 抛出 std::out_of_range
     */
    auto ReadFixedInt64() -> int64_t;

    /**
     * @brief 读取uint64_t类型的数据
     * @pre getReadSize() >= sizeof(uint64_t)
     * @post m_position += sizeof(uint64_t);
     * @exception 如果getReadSize() < sizeof(uint64_t) 抛出 std::out_of_range
     */
    auto ReadFixedUint64() -> uint64_t;

    /**
     * @brief 读取float类型的数据
     * @pre getReadSize() >= sizeof(float)
     * @post m_position += sizeof(float);
     * @exception 如果getReadSize() < sizeof(float) 抛出 std::out_of_range
     */
    auto ReadFixedFloat() -> float;

    /**
     * @brief 读取double类型的数据
     * @pre getReadSize() >= sizeof(double)
     * @post m_position += sizeof(double);
     * @exception 如果getReadSize() < sizeof(double) 抛出 std::out_of_range
     */
    auto ReadFixedDouble() -> double;

    /**
     * @brief 读取有符号Varint32类型的数据
     * @pre getReadSize() >= 有符号Varint32实际占用内存
     * @post m_position += 有符号Varint32实际占用内存
     * @exception 如果getReadSize() < 有符号Varint32实际占用内存 抛出
     * std::out_of_range
     */
    auto ReadInt32() -> int32_t;

    /**
     * @brief 读取无符号Varint32类型的数据
     * @pre getReadSize() >= 无符号Varint32实际占用内存
     * @post m_position += 无符号Varint32实际占用内存
     * @exception 如果getReadSize() < 无符号Varint32实际占用内存 抛出
     * std::out_of_range
     */
    auto ReadUint32() -> uint32_t;

    /**
     * @brief 读取有符号Varint64类型的数据
     * @pre getReadSize() >= 有符号Varint64实际占用内存
     * @post m_position += 有符号Varint64实际占用内存
     * @exception 如果getReadSize() < 有符号Varint64实际占用内存 抛出
     * std::out_of_range
     */
    auto ReadInt64() -> int64_t;

    /**
     * @brief 读取无符号Varint64类型的数据
     * @pre getReadSize() >= 无符号Varint64实际占用内存
     * @post m_position += 无符号Varint64实际占用内存
     * @exception 如果getReadSize() < 无符号Varint64实际占用内存 抛出
     * std::out_of_range
     */
    auto ReadUint64() -> uint64_t;

    /**
     * @brief 读取std::string类型的数据,用uint16_t作为长度
     * @pre getReadSize() >= sizeof(uint16_t) + size
     * @post m_position += sizeof(uint16_t) + size;
     * @exception 如果getReadSize() < sizeof(uint16_t) + size 抛出
     * std::out_of_range
     */
    auto ReadStringFixedUint16() -> std::string;

    /**
     * @brief 读取std::string类型的数据,用uint32_t作为长度
     * @pre getReadSize() >= sizeof(uint32_t) + size
     * @post m_position += sizeof(uint32_t) + size;
     * @exception 如果getReadSize() < sizeof(uint32_t) + size 抛出
     * std::out_of_range
     */
    auto ReadStringFixedUint32() -> std::string;

    /**
     * @brief 读取std::string类型的数据,用uint64_t作为长度
     * @pre getReadSize() >= sizeof(uint64_t) + size
     * @post m_position += sizeof(uint64_t) + size;
     * @exception 如果getReadSize() < sizeof(uint64_t) + size 抛出
     * std::out_of_range
     */
    auto ReadStringFixedUint64() -> std::string;

    /**
     * @brief 读取std::string类型的数据,用无符号Varint64作为长度
     * @pre getReadSize() >= 无符号Varint64实际大小 + size
     * @post m_position += 无符号Varint64实际大小 + size;
     * @exception 如果getReadSize() < 无符号Varint64实际大小 + size 抛出
     * std::out_of_range
     */
    auto ReadStringVint64() -> std::string;

    /**
     * @brief 清空ByteArray
     * @post m_position = 0, m_size = 0
     */
    void Clear();

    /**
     * @brief 写入size长度的数据
     * @param[in] buf 内存缓存指针
     * @param[in] size 数据大小
     * @post m_position += size, 如果m_position > m_size 则 m_size = m_position
     */
    void Write(const void* buf, size_t size);

    /**
     * @brief
     * 读取size长度的数据，从m_position开始读，一般第一次读之前会将m_position设为0
     * @param[out] buf 内存缓存指针
     * @param[in] size 数据大小
     * @post m_position += size, 如果m_position > m_size 则 m_size = m_position
     * @exception 如果getReadSize() < size 则抛出 std::out_of_range
     */
    void Read(void* buf, size_t size);

    /**
     * @brief 读取size长度的数据
     * @param[out] buf 内存缓存指针
     * @param[in] size 数据大小
     * @param[in] position 读取开始位置
     * @exception 如果 (m_size - position) < size 则抛出 std::out_of_range
     */
    void Read(void* buf, size_t size, size_t position) const;

    /**
     * @brief 返回ByteArray当前位置
     */
    auto GetPosition() const -> size_t;

    /**
     * @brief 设置ByteArray当前位置
     * @post 如果m_position > m_size 则 m_size = m_position
     * @exception 如果m_position > m_capacity 则抛出 std::out_of_range
     */
    void SetPosition(size_t value);

    /**
     * @brief 把ByteArray的数据写入到文件中
     * @param[in] name 文件名
     */
    bool WriteToFile(const std::string& name) const;

    /**
     * @brief 从文件中读取数据
     * @param[in] name 文件名
     */
    bool ReadFromFile(const std::string& name);

    /**
     * @brief 返回内存块的大小
     */
    auto GetBaseSize() const -> size_t;

    /**
     * @brief 返回可读取数据大小
     */
    auto GetReadableSize() const -> size_t;

    /**
     * @brief 是否是小端
     */
    bool IsLittleEndian() const;

    /**
     * @brief 设置是否为小端
     */
    void SetIsLittleEndian(bool val);

    /**
     * @brief 将ByteArray里面的数据[m_position, m_size)转成std::string
     */
    auto ToString() const -> std::string;

    /**
     * @brief 将ByteArray里面的数据[m_position,
     * m_size)转成16进制的std::string(格式:FF FF FF)
     */
    auto ToHexString() const -> std::string;

    /**
     * @brief 获取可读取的缓存,保存成iovec数组
     * @param[out] buffers 保存可读取数据的iovec数组
     * @param[in] len 读取数据的长度,如果len > getReadSize() 则 len =
     * getReadSize()
     * @return 返回实际数据的长度
     */
    auto GetReadBuffers(std::vector<iovec>& buffers, size_t len = ~0ULL) const
        -> size_t;

    /**
     * @brief 获取可读取的缓存,保存成iovec数组,从position位置开始
     * @param[out] buffers 保存可读取数据的iovec数组
     * @param[in] len 读取数据的长度,如果len > getReadSize() 则 len =
     * getReadSize()
     * @param[in] position 读取数据的位置
     * @return 返回实际数据的长度
     */
    auto GetReadBuffers(std::vector<iovec>& buffers, size_t len,
                        size_t position) const -> size_t;

    /**
     * @brief 获取可写入的缓存,保存成iovec数组
     * @param[out] buffers 保存可写入的内存的iovec数组
     * @param[in] len 写入的长度
     * @return 返回实际的长度
     * @post 如果(m_position + len) > m_capacity 则
     * m_capacity扩容N个节点以容纳len长度
     */
    auto GetWriteBuffers(std::vector<iovec>& buffers, size_t len) -> size_t;

    /**
     * @brief 返回数据的长度
     */
    auto GetSize() const -> size_t;

  private:
    /**
     * @brief
     * 扩容ByteArray,使其可以容纳size个数据(如果原本可以可以容纳,则不扩容)
     */
    void AddWritableCapacity(size_t size);

    /**
     * @brief 获取当前的可写入容量
     */
    auto GetWritableCapacity() const -> size_t;

  private:
    size_t m_base_size{};  // 内存块的大小
    size_t m_position{0};  // 当前操作位置（读写从此处开始）
    size_t m_capacity{};   // 数据写入的上限
    size_t m_size{0};      // 当前数据的大小
    uint16_t m_endian{};   // 字节序,默认大端
    Node* m_root{};        // 第一个内存块指针
    Node* m_cur{};         // 当前操作的内存块指针
};
}  // namespace wtsclwq
