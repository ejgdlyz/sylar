#include "sylar/bytearray.h"
#include "sylar/sylar.h"
#include "sylar/log.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test() {
#define XX(type, len, write_fun, read_fun, base_len) \
    std::vector<type> vec; \
    for (int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));  \
    for(auto& v : vec) { \
        ba->write_fun(v); \
    } \
    ba->setPosition(0); \
    for (size_t i = 0; i < vec.size(); ++i) {  \
        int32_t v = ba->read_fun(); \
        SYLAR_LOG_INFO(g_logger) << i << " - " << v << " - " << (int32_t)vec[i];\
        SYLAR_ASSERT(v == vec[i]); \
    } \
    SYLAR_ASSERT(ba->getReadSize() == 0); \
    SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type ") len=" \
        << len << " base_len=" << base_len;

    XX(int8_t, 100, writeFint8, readFint8, 1);

#undef XX
}

void test_bytearray() {
#define XX(type, len, write_fun, read_fun, base_len) {\
    std::vector<type> vec; \
    for (int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));  \
    for(auto& v : vec) { \
        ba->write_fun(v); \
    } \
    ba->setPosition(0); \
    for (size_t i = 0; i < vec.size(); ++i) {  \
        type v = ba->read_fun(); \
        SYLAR_ASSERT(v == vec[i]); \
    } \
    SYLAR_ASSERT(ba->getReadSize() == 0); \
    SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type ") len=" \
        << len << " base_len=" << base_len \
        << " size=" << ba->getSize();  \
}

    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeInt32, readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t, 100, writeInt64, readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);
#undef XX

// 文件测试
SYLAR_LOG_INFO(g_logger) << " ****** File test ******";

#define XX(type, len, write_fun, read_fun, base_len) {\
    std::vector<type> vec; \
    for (int i = 0; i < len; ++i) { \
        vec.push_back(rand()); \
    } \
    sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));  \
    for(auto& v : vec) { \
        ba->write_fun(v); \
    } \
    ba->setPosition(0); \
    for (size_t i = 0; i < vec.size(); ++i) {  \
        type v = ba->read_fun(); \
        SYLAR_ASSERT(v == vec[i]); \
    } \
    SYLAR_ASSERT(ba->getReadSize() == 0); \
    SYLAR_LOG_INFO(g_logger) << #write_fun "/" #read_fun " (" #type ") len=" \
        << len << " base_len=" << base_len \
        << " size=" << ba->getSize();  \
    ba->setPosition(0);  \
    SYLAR_ASSERT(ba->writeToFile("/tmp/" #type "_" #len "_" #read_fun ".dat")); \
    sylar::ByteArray::ptr ba2(new sylar::ByteArray(base_len * 2)); \
    SYLAR_ASSERT(ba2->readFromFile("/tmp/" #type "_" #len "_" #read_fun ".dat")); \
    ba2->setPosition(0);  \
    SYLAR_ASSERT(ba->toString() == ba2->toString()); \
    SYLAR_ASSERT(ba->getPosition() == 0); \
    SYLAR_ASSERT(ba2->getPosition() == 0); \
}

    XX(int8_t, 100, writeFint8, readFint8, 1);
    XX(uint8_t, 100, writeFuint8, readFuint8, 1);
    XX(int16_t, 100, writeFint16, readFint16, 1);
    XX(uint16_t, 100, writeFuint16, readFuint16, 1);
    XX(int32_t, 100, writeFint32, readFint32, 1);
    XX(uint32_t, 100, writeFuint32, readFuint32, 1);
    XX(int64_t, 100, writeFint64, readFint64, 1);
    XX(uint64_t, 100, writeFuint64, readFuint64, 1);

    XX(int32_t, 100, writeInt32, readInt32, 1);
    XX(uint32_t, 100, writeUint32, readUint32, 1);
    XX(int64_t, 100, writeInt64, readInt64, 1);
    XX(uint64_t, 100, writeUint64, readUint64, 1);
#undef XX

}

void test_debug() {
    std::vector<int8_t> vec; 
    int len = 10;
    int base_len = 3;
    for (int i = 0; i < len; ++i) { 
        vec.push_back(i); 
    } 
    sylar::ByteArray::ptr ba(new sylar::ByteArray(base_len));
    
    for(auto& v : vec) { 
        ba->writeFint32(v);  // 4 字节，需要两个块存储
    } 

    ba->setPosition(0); 
    for (size_t i = 0; i < vec.size(); ++i) {  
        int32_t v = ba->readFint32(); 
        SYLAR_LOG_INFO(g_logger) << i << " - " << v << " - " << (int32_t)vec[i];
        SYLAR_ASSERT(v == vec[i]); 
    } 
    SYLAR_ASSERT(ba->getReadSize() == 0); 
    SYLAR_LOG_INFO(g_logger) << " len=" << len << " base_len=" << base_len;

    // XX(int8_t, 100, writeFint8, readFint8, 1);
}

int main(int argc, char const *argv[])
{
    test_bytearray();
    // test_debug();
    return 0;
}
