//
// Created by wtsclwq on 23-4-1.
//

#include "../include/serialize/byte_array.h"

#include <cstring>
#include <fstream>
#include <stdexcept>

#include "../include/log/log_manager.h"
#include "../include/util/net_util.h"
namespace wtsclwq {
static Logger::ptr sys_logger = GET_LOGGER_BY_NAME("system");

ByteArray::Node::Node(size_t size) : size(size), ptr(new char[size]) {}

ByteArray::Node::~Node() { delete[] ptr; }

ByteArray::ByteArray()
    : m_base_size(4096), m_capacity(4096), m_endian(BIG_ENDIAN),
      m_root(new Node(4096)), m_cur(m_root) {}

ByteArray::ByteArray(size_t base_size)
    : m_base_size(base_size), m_capacity(base_size), m_endian(BIG_ENDIAN),
      m_root(new Node(base_size)), m_cur(m_root) {}

ByteArray::~ByteArray() {
    Node *temp = m_root;
    while (temp != nullptr) {
        m_cur = temp;
        temp = temp->next;
        delete m_cur;
    }
}

void ByteArray::WriteFixedInt8(int8_t value) { Write(&value, sizeof(value)); }

void ByteArray::WriteFixedUint8(uint8_t value) { Write(&value, sizeof(value)); }

void ByteArray::WriteFixedInt16(int16_t value) {
    if (m_endian != BYTE_ORDER) {
        value = ByteSwap(value);
    }
    Write(&value, sizeof(value));
}

void ByteArray::WriteFixedUint16(uint16_t value) {
    if (m_endian != BYTE_ORDER) {
        value = ByteSwap(value);
    }
    Write(&value, sizeof(value));
}

void ByteArray::WriteFixedInt32(int32_t value) {
    if (m_endian != BYTE_ORDER) {
        value = ByteSwap(value);
    }
    Write(&value, sizeof(value));
}

void ByteArray::WriteFixedUint32(uint32_t value) {
    if (m_endian != BYTE_ORDER) {
        value = ByteSwap(value);
    }
    Write(&value, sizeof(value));
}

void ByteArray::WriteFixedInt64(int64_t value) {
    if (m_endian != BYTE_ORDER) {
        value = ByteSwap(value);
    }
    Write(&value, sizeof(value));
}

void ByteArray::WriteFixedUint64(uint64_t value) {
    if (m_endian != BYTE_ORDER) {
        value = ByteSwap(value);
    }
    Write(&value, sizeof(value));
}

void ByteArray::WriteFixedFloat(float value) {
    uint32_t val;
    memcpy(&val, &value, sizeof(value));
    WriteFixedUint32(val);
}

void ByteArray::WriteFixedDouble(double value) {
    uint64_t val;
    memcpy(&val, &value, sizeof(value));
    WriteFixedUint32(val);
}

inline auto ZigZagEncode32(int32_t n) -> uint32_t {
    // Note:  the right-shift must be arithmetic
    return (n << 1) ^ (n >> 31);
}

inline auto ZigZagDecode32(uint32_t n) -> int32_t {
    return (n >> 1) ^ -static_cast<int32_t>(n & 1);
}

inline auto ZigZagEncode64(int64_t n) -> uint64_t {
    // Note:  the right-shift must be arithmetic
    return (n << 1) ^ (n >> 63);
}

inline auto ZigZagDecode64(uint64_t n) -> int64_t {
    return (n >> 1) ^ -static_cast<int64_t>(n & 1);
}

void ByteArray::WriteInt32(int32_t value) {
    WriteUint32(ZigZagEncode32(value));
}

void ByteArray::WriteUint32(uint32_t value) {
    // 最多5个字节(40bit)，32/7 = 4...4
    uint8_t temp[5];
    uint8_t i;
    // 如果数据大于一个字节(127是一个字节最大数据),那么继续,即需要在最高位加上1
    for (i = 0; value > 127; ++i) {
        // value&0x7F表示取出下7bit数据, 0x80表示在最高位加上1
        temp[i] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    temp[i] = value;
    i++;
    Write(temp, i);
}

void ByteArray::WriteInt64(int64_t value) {
    WriteUint64(ZigZagEncode64(value));
}

void ByteArray::WriteUint64(uint64_t value) {
    // 最多10个字节（80bit），64/7 = 9...1
    uint8_t temp[10];
    uint8_t i;
    // 如果数据大于一个字节(127是一个字节最大数据),那么继续,即需要在最高位加上1
    for (i = 0; value > 127; ++i) {
        // value&0x7F表示取出下7bit数据, 0x80表示在最高位加上1
        temp[i] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    temp[i] = static_cast<uint8_t>(value);
    i++;
    Write(temp, i);
}

void ByteArray::WriteStringFixedUint16(const std::string &value) {
    WriteFixedUint16(value.size());
    Write(value.c_str(), value.size());
}

void ByteArray::WriteStringFixedUint32(const std::string &value) {
    WriteFixedUint32(value.size());
    Write(value.c_str(), value.size());
}

void ByteArray::WriteStringFixedUint64(const std::string &value) {
    WriteFixedUint64(value.size());
    Write(value.c_str(), value.size());
}

void ByteArray::WriteStringVint(const std::string &value) {
    WriteUint64(value.size());
    Write(value.c_str(), value.size());
}

void ByteArray::WriteStringWithoutLength(const std::string &value) {
    Write(value.c_str(), value.size());
}

auto ByteArray::ReadFixedInt8() -> int8_t {
    int8_t value;
    Read(&value, sizeof(value));
    return value;
}

auto ByteArray::ReadFixedUint8() -> uint8_t {
    uint8_t value;
    Read(&value, sizeof(value));
    return value;
}

auto ByteArray::ReadFixedInt16() -> int16_t {
    int16_t value;
    Read(&value, sizeof(value));
    if (m_endian == BYTE_ORDER) {
        return value;
    }
    return ByteSwap(value);
}

auto ByteArray::ReadFixedUint16() -> uint16_t {
    uint16_t value;
    Read(&value, sizeof(value));
    if (m_endian == BYTE_ORDER) {
        return value;
    }
    return ByteSwap(value);
}

auto ByteArray::ReadFixedInt32() -> int32_t {
    int32_t value;
    Read(&value, sizeof(value));
    if (m_endian == BYTE_ORDER) {
        return value;
    }
    return ByteSwap(value);
}

auto ByteArray::ReadFixedUint32() -> uint32_t {
    uint32_t value;
    Read(&value, sizeof(value));
    if (m_endian == BYTE_ORDER) {
        return value;
    }
    return ByteSwap(value);
}
auto ByteArray::ReadFixedInt64() -> int64_t {
    int64_t value;
    Read(&value, sizeof(value));
    if (m_endian == BYTE_ORDER) {
        return value;
    }
    return ByteSwap(value);
}

auto ByteArray::ReadFixedUint64() -> uint64_t {
    uint64_t value;
    Read(&value, sizeof(value));
    if (m_endian == BYTE_ORDER) {
        return value;
    }
    return ByteSwap(value);
}

auto ByteArray::ReadFixedFloat() -> float {
    uint32_t val = ReadFixedUint32();
    float value;
    memcpy(&value, &val, sizeof(val));
    return value;
}

auto ByteArray::ReadFixedDouble() -> double {
    uint64_t val = ReadFixedUint32();
    double value;
    memcpy(&value, &val, sizeof(val));
    return value;
}

auto ByteArray::ReadInt32() -> int32_t { return ZigZagDecode32(ReadUint32()); }

auto ByteArray::ReadUint32() -> uint32_t {
    uint32_t result = 0;
    for (int i = 0; i < 32; i += 7) {
        uint8_t byte = ReadFixedUint8();
        if (byte < 128) {
            result |= (static_cast<uint32_t>(byte) << i);
            break;
        }
        // 读取字节内的低7位，最高标志位丢弃
        result |= (static_cast<uint32_t>(byte & 0x7F) << i);
    }
    return result;
}

auto ByteArray::ReadInt64() -> int64_t { return ZigZagDecode64(ReadUint64()); }

auto ByteArray::ReadUint64() -> uint64_t {
    uint64_t result = 0;
    for (int i = 0; i < 64; i += 7) {
        uint8_t byte = ReadFixedUint8();
        if (byte < 128) {
            result |= (static_cast<uint64_t>(byte) << i);
            break;
        }
        // 读取字节内的低7位，最高标志位丢弃
        result |= (static_cast<uint64_t>(byte & 0x7F) << i);
    }
    return result;
}

auto ByteArray::ReadStringFixedUint16() -> std::string {
    uint16_t len = ReadFixedUint16();
    std::string buffer;
    buffer.resize(len);
    Read(buffer.data(), len);
    return buffer;
}

auto ByteArray::ReadStringFixedUint32() -> std::string {
    uint32_t len = ReadFixedUint16();
    std::string buffer;
    buffer.resize(len);
    Read(buffer.data(), len);
    return buffer;
}

auto ByteArray::ReadStringFixedUint64() -> std::string {
    uint64_t len = ReadFixedUint16();
    std::string buffer;
    buffer.resize(len);
    Read(buffer.data(), len);
    return buffer;
}

auto ByteArray::ReadStringVint64() -> std::string {
    uint64_t len = ReadUint64();
    std::string buffer;
    buffer.resize(len);
    Read(buffer.data(), len);
    return buffer;
}

void ByteArray::Clear() {
    m_size = 0;
    m_position = 0;
    m_capacity = m_base_size;
    Node *temp = m_root->next;
    while (temp != nullptr) {
        m_cur = temp;
        temp = temp->next;
        delete m_cur;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::Write(const void *buf, size_t size) {
    if (size == 0) {
        return;
    }
    AddWritableCapacity(size);
    size_t cur_node_pos = m_position % m_base_size;  // 当前节点可以写入的位置
    size_t cur_node_cap = m_cur->size - cur_node_pos;  // 当前节点的剩余容量
    size_t buf_position = 0;                           // buffer起始位置
    while (size > 0) {
        // 如果剩余容量能够存下buf
        if (cur_node_cap >= size) {
            memcpy(m_cur->ptr + cur_node_pos,
                   reinterpret_cast<const char *>(buf) + buf_position, size);
            // 如果当前node的剩余容量刚好用完
            if (m_cur->size == (cur_node_pos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            size = 0;
        } else {
            // 用完当前node的剩余容量
            memcpy(m_cur->ptr + cur_node_pos,
                   reinterpret_cast<const char *>(buf) + buf_position,
                   cur_node_cap);
            m_position += cur_node_cap;    // 将总位置向后移动
            buf_position += cur_node_cap;  // 更新buffer起始位置
            size -= cur_node_cap;          // 更新buffer剩余数据量
            m_cur = m_cur->next;           // 当前节点已满，向后移动
            cur_node_cap = m_cur->size;    // 新的当前节点为空
            cur_node_pos = 0;  // 新的当前节点可写入的位置
        }
    }
    // 更新总数据量
    if (m_position > m_size) {
        m_size = m_position;
    }
}

void ByteArray::Read(void *buf, size_t size) {
    if (size > GetReadableSize()) {
        throw std::out_of_range("not enough len");
    }
    size_t cur_node_pos = m_position % m_base_size;  // 当前节点可以读取的位置
    size_t cur_node_cap = m_cur->size - cur_node_pos;  // 当前节点的可读容量
    size_t buf_position = 0;                           // buffer起始位置
    while (size > 0) {
        // 如果当前节点的可读容量能够满足buf的需求
        if (cur_node_cap >= size) {
            memcpy(reinterpret_cast<char *>(buf) + buf_position,
                   m_cur->ptr + cur_node_pos, size);
            // 如果当前node的可读容量刚好用完
            if (m_cur->size == (cur_node_pos + size)) {
                m_cur = m_cur->next;
            }
            m_position += size;
            size = 0;
        } else {
            // 用完当前node的可读容量
            memcpy(reinterpret_cast<char *>(buf) + buf_position,
                   m_cur->ptr + cur_node_pos, cur_node_cap);
            m_position += cur_node_cap;    // 将总位置向后移动
            buf_position += cur_node_cap;  // 更新buffer起始位置
            size -= cur_node_cap;          // 更新buffer剩余需求量
            m_cur = m_cur->next;           // 当前节点已满，向后移动
            cur_node_cap = m_cur->size;    // 新的当前节点为空
            cur_node_pos = 0;              // 新的当前节点可读的位置
        }
    }
}

void ByteArray::Read(void *buf, size_t size, size_t position) const {
    if (size > (m_size - position)) {
        throw std::out_of_range("not enough len");
    }
    Node *cur = m_cur;
    size_t buf_position = 0;
    size_t cur_node_pos = position % m_base_size;  // 当前节点可以读取的位置
    size_t count = position / m_base_size;
    while (count > 0) {
        cur = cur->next;
        --count;
    }
    size_t cur_node_cap = cur->size - cur_node_pos;  // 当前节点的可读容量
    // buffer起始位置
    while (size > 0) {
        // 如果当前节点的可读容量能够满足buf的需求
        if (cur_node_cap >= size) {
            memcpy(reinterpret_cast<char *>(buf) + buf_position,
                   cur->ptr + cur_node_pos, size);
            // 如果当前node的可读容量刚好用完
            if (cur->size == (cur_node_pos + size)) {
                cur = cur->next;
            }
            position += size;
            size = 0;
        } else {
            // 用完当前node的可读容量
            memcpy(reinterpret_cast<char *>(buf) + buf_position,
                   cur->ptr + cur_node_pos, cur_node_cap);
            position += cur_node_cap;      // 将总位置向后移动
            buf_position += cur_node_cap;  // 更新buffer起始位置
            size -= cur_node_cap;          // 更新buffer剩余需求量
            cur = cur->next;               // 当前节点已满，向后移动
            cur_node_cap = cur->size;      // 新的当前节点为空
            cur_node_pos = 0;              // 新的当前节点可读的位置
        }
    }
}

auto ByteArray::GetPosition() const -> size_t { return m_position; }

void ByteArray::SetPosition(size_t value) {
    if (value > m_capacity) {
        throw std::out_of_range("SetPosition out of range");
    }
    m_position = value;
    if (m_position > m_size) {
        m_size = m_position;
    }
    m_cur = m_root;
    while (value > m_cur->size) {
        value -= m_cur->size;
        m_cur = m_cur->next;
    }
    if (value == m_cur->size) {
        m_cur = m_cur->next;
    }
}

auto ByteArray::WriteToFile(const std::string &name) const -> bool {
    std::ofstream ofs;
    ofs.open(name, std::ios::trunc | std::ios::binary);
    if (!ofs) {
        LOG_CUSTOM_ERROR(sys_logger,
                         "WriteToFile name = %s,error, errno = %d, errstr = %s",
                         name.c_str(), errno, strerror(errno))
        return false;
    }
    int64_t read_size = GetReadableSize();
    int64_t pos = m_position;
    Node *cur = m_cur;

    while (read_size > 0) {
        int diff = pos % m_base_size;
        int64_t len =
            (read_size > static_cast<int64_t>(m_base_size) ? m_base_size
                                                           : read_size) -
            diff;
        ofs.write(cur->ptr + diff, len);
        cur = cur->next;
        pos += len;
        read_size -= len;
    }
    return true;
}

auto ByteArray::ReadFromFile(const std::string &name) -> bool {
    std::ifstream ifs;
    ifs.open(name, std::ios::binary);
    if (!ifs) {
        LOG_CUSTOM_ERROR(
            sys_logger, "ReadFromFile name = %s,error, errno = %d, errstr = %s",
            name.c_str(), errno, strerror(errno))
        return false;
    }

    std::shared_ptr<char> buff(new char[m_base_size],
                               [](const char *ptr) { delete[] ptr; });
    while (!ifs.eof()) {
        ifs.read(buff.get(), m_base_size);
        Write(buff.get(), ifs.gcount());
    }
    return true;
}

auto ByteArray::GetBaseSize() const -> size_t { return m_base_size; }

auto ByteArray::GetReadableSize() const -> size_t {
    return m_size - m_position;
}

bool ByteArray::IsLittleEndian() const { return m_endian == LITTLE_ENDIAN; }

void ByteArray::SetIsLittleEndian(bool val) {
    m_endian = val ? LITTLE_ENDIAN : BIG_ENDIAN;
}

auto ByteArray::ToString() const -> std::string {
    std::string str;
    str.resize(GetReadableSize());
    if (str.empty()) {
        return str;
    }
    Read(str.data(), str.size(), m_position);
    return str;
}

auto ByteArray::ToHexString() const -> std::string {
    std::string str = ToString();
    std::stringstream ss;
    for (auto i = 0; i < str.size(); ++i) {
        if (i > 0 && i % 32 == 0) {
            ss << std::endl;
        }
        ss << std::setw(2) << std::setfill('0') << std::hex
           << static_cast<int>(static_cast<uint8_t>(str[i])) << " ";
    }
    return ss.str();
}

auto ByteArray::GetReadBuffers(std::vector<iovec> &buffers, size_t len) const
    -> size_t {
    len = len > GetReadableSize() ? GetReadableSize() : len;
    if (len == 0) {
        return 0;
    }
    Node *cur = m_cur;
    size_t size = len;
    size_t cur_node_pos = m_position % m_base_size;
    size_t cur_node_cap = cur->size - cur_node_pos;
    struct iovec iov;
    while (len != 0) {
        if (cur_node_cap >= len) {
            iov.iov_base = cur->ptr + cur_node_pos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + cur_node_pos;
            iov.iov_len = cur_node_cap;
            len -= cur_node_cap;
            cur = cur->next;
            cur_node_cap = cur->size;
            cur_node_pos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}
auto ByteArray::GetReadBuffers(std::vector<iovec> &buffers, size_t len,
                               size_t position) const -> size_t {
    len = len > GetReadableSize() ? GetReadableSize() : len;
    if (len == 0) {
        return 0;
    }
    uint64_t size = len;
    size_t cur_node_pos = position % m_base_size;
    size_t count = position / m_base_size;
    Node *cur = m_root;
    while (count > 0) {
        cur = cur->next;
        --count;
    }
    size_t cur_node_cap = cur->size - cur_node_pos;
    struct iovec iov;
    while (len > 0) {
        if (cur_node_cap >= len) {
            iov.iov_base = cur->ptr + cur_node_pos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + cur_node_pos;
            iov.iov_len = cur_node_cap;
            len -= cur_node_cap;
            cur = cur->next;
            cur_node_cap = cur->size;
            cur_node_pos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}

auto ByteArray::GetWriteBuffers(std::vector<iovec> &buffers, size_t len)
    -> size_t {
    if (len == 0) {
        return 0;
    }
    AddWritableCapacity(len);
    uint64_t size = len;

    size_t cur_node_pos = m_position % m_base_size;
    size_t cur_node_cap = m_cur->size - cur_node_pos;
    struct iovec iov;
    Node *cur = m_cur;
    while (len > 0) {
        if (cur_node_cap >= len) {
            iov.iov_base = cur->ptr + cur_node_pos;
            iov.iov_len = len;
            len = 0;
        } else {
            iov.iov_base = cur->ptr + cur_node_pos;
            iov.iov_len = cur_node_cap;

            len -= cur_node_cap;
            cur = cur->next;
            cur_node_cap = cur->size;
            cur_node_pos = 0;
        }
        buffers.push_back(iov);
    }
    return size;
}
auto ByteArray::GetSize() const -> size_t { return m_size; }

void ByteArray::AddWritableCapacity(size_t size) {
    if (size == 0) {
        return;
    }
    size_t old_cap = GetWritableCapacity();
    if (old_cap >= size) {
        return;
    }

    size = size - old_cap;
    size_t count = ceil(1.0 * size / m_base_size);
    Node *temp = m_root;
    while (temp->next != nullptr) {
        temp = temp->next;
    }
    Node *first = nullptr;
    for (auto i = 0; i < count; ++i) {
        temp->next = new Node(m_base_size);
        if (first == nullptr) {
            first = temp->next;
        }
        temp = temp->next;
        m_capacity += m_base_size;
    }
    if (old_cap == 0) {
        m_cur = first;
    }
}

auto ByteArray::GetWritableCapacity() const -> size_t {
    return m_capacity - m_position;
}
}  // namespace wtsclwq
