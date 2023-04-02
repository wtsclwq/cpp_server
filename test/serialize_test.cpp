//
// Created by wtsclwq on 23-4-2.
//

#include "../src/include/log/log_manager.h"
#include "../src/include/serialize/byte_array.h"
#include "../src/include/util/macro.h"

static wtsclwq::Logger::ptr logger = ROOT_LOGGER;
void test() {
#define TEST_BYTE_ARR(type, len, write_fun, read_fun, base_len)             \
    {                                                                       \
        std::vector<type> vec;                                              \
        for (int i = 0; i < (len); ++i) {                                   \
            vec.push_back(rand());                                          \
        }                                                                   \
        wtsclwq::ByteArray::ptr ba(new wtsclwq::ByteArray(base_len));       \
        for (auto &i : vec) {                                               \
            ba->write_fun(i);                                               \
        }                                                                   \
        ba->SetPosition(0);                                                 \
        for (size_t i = 0; i < vec.size(); ++i) {                           \
            type v = ba->read_fun();                                        \
            WTSCLWQ_ASSERT(v == vec[i], "value not equal");                 \
        }                                                                   \
        WTSCLWQ_ASSERT(ba->GetReadableSize() == 0,                          \
                       "readable size is not zero");                        \
        LOG_CUSTOM_INFO(                                                    \
            logger, "%s, --  %s, (%s) len = %d, base_len = %d, size = %lu", \
            #write_fun, #read_fun, #type, len, base_len, ba->GetSize())     \
    }
    TEST_BYTE_ARR(int8_t, 100, WriteFixedInt8, ReadFixedInt8, 4096);

    TEST_BYTE_ARR(uint8_t, 100, WriteFixedUint8, ReadFixedUint8, 4096);

    TEST_BYTE_ARR(int16_t, 100, WriteFixedInt16, ReadFixedInt16, 4096);

    TEST_BYTE_ARR(uint16_t, 100, WriteFixedUint16, ReadFixedUint16, 4096);

    TEST_BYTE_ARR(int32_t, 100, WriteFixedInt32, ReadFixedInt32, 1);

    TEST_BYTE_ARR(uint32_t, 100, WriteFixedUint32, ReadFixedUint32, 1);

    TEST_BYTE_ARR(int64_t, 100, WriteFixedInt64, ReadFixedInt64, 1);

    TEST_BYTE_ARR(uint64_t, 100, WriteFixedUint64, ReadFixedUint64, 1);

    TEST_BYTE_ARR(int32_t, 100, WriteInt32, ReadInt32, 1);

    TEST_BYTE_ARR(uint32_t, 100, WriteUint32, ReadUint32, 1);

    TEST_BYTE_ARR(int64_t, 100, WriteInt64, ReadInt64, 1);

    TEST_BYTE_ARR(uint64_t, 100, WriteUint64, ReadUint64, 1);

#undef TEST_BYTE_ARR
}

void test_file() {
#define TEST_BYTE_ARR(type, len, write_fun, read_fun, base_len)              \
    {                                                                        \
        std::vector<type> vec;                                               \
        for (int i = 0; i < (len); ++i) {                                    \
            vec.push_back(rand());                                           \
        }                                                                    \
        wtsclwq::ByteArray::ptr ba(new wtsclwq::ByteArray(base_len));        \
        for (auto &i : vec) {                                                \
            ba->write_fun(i);                                                \
        }                                                                    \
        ba->SetPosition(0);                                                  \
        for (size_t i = 0; i < vec.size(); ++i) {                            \
            type v = ba->read_fun();                                         \
            WTSCLWQ_ASSERT(v == vec[i], "value not equal");                  \
        }                                                                    \
        WTSCLWQ_ASSERT(ba->GetReadableSize() == 0,                           \
                       "readable size is not zero");                         \
        LOG_CUSTOM_INFO(                                                     \
            logger, "%s, --  %s, (%s) len = %d, base_len = %d, size = %lu",  \
            #write_fun, #read_fun, #type, len, base_len, ba->GetSize())      \
        ba->SetPosition(0);                                                  \
        WTSCLWQ_ASSERT(                                                      \
            ba->WriteToFile("/tmp/" #type "_" #len "-" #read_fun ".dat"),    \
            "write to file error");                                          \
        wtsclwq::ByteArray::ptr ba2(new wtsclwq::ByteArray((base_len)*2));   \
        WTSCLWQ_ASSERT(                                                      \
            ba2->ReadFromFile("/tmp/" #type "_" #len "-" #read_fun ".dat"),  \
            "read from file error");                                         \
        ba2->SetPosition(0);                                                 \
        WTSCLWQ_ASSERT(ba->ToString() == ba2->ToString(), "tostring error"); \
        WTSCLWQ_ASSERT(ba->GetPosition() == 0, "get position1 error");       \
        WTSCLWQ_ASSERT(ba2->GetPosition() == 0, "get position2 error");      \
        for (size_t i = 0; i < vec.size(); ++i) {                            \
            type v = ba->read_fun();                                         \
            LOG_CUSTOM_INFO(logger, "val = %ld", v);                          \
            WTSCLWQ_ASSERT(v == vec[i], "value not equal");                  \
        }                                                                    \
    }
//    TEST_BYTE_ARR(int8_t, 100, WriteFixedInt8, ReadFixedInt8, 1);

//    TEST_BYTE_ARR(uint8_t, 100, WriteFixedUint8, ReadFixedUint8, 1);
//
//    TEST_BYTE_ARR(int16_t, 100, WriteFixedInt16, ReadFixedInt16, 1);
//
//    TEST_BYTE_ARR(uint16_t, 100, WriteFixedUint16, ReadFixedUint16, 1);
//
//    TEST_BYTE_ARR(int32_t, 100, WriteFixedInt32, ReadFixedInt32, 1);
//
//    TEST_BYTE_ARR(uint32_t, 100, WriteFixedUint32, ReadFixedUint32, 1);
//
    TEST_BYTE_ARR(int64_t, 100, WriteFixedInt64, ReadFixedInt64, 4096);
//
//    TEST_BYTE_ARR(uint64_t, 100, WriteFixedUint64, ReadFixedUint64, 1);
//
//    TEST_BYTE_ARR(int32_t, 100, WriteInt32, ReadInt32, 1);
//
//    TEST_BYTE_ARR(uint32_t, 100, WriteUint32, ReadUint32, 1);
//
//    TEST_BYTE_ARR(int64_t, 100, WriteInt64, ReadInt64, 1);
//
//    TEST_BYTE_ARR(uint64_t, 100, WriteUint64, ReadUint64, 1);
#undef TEST_BYTE_ARR
}
auto main() -> int {
    test_file();
    return 0;
}