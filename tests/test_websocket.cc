#include "sylar/util/hash_util.h"
#include <iostream>
#include <string>

void test_base64encode() {
    std::string str_encode = sylar::base64encode("abc");
    std::string str_decode = sylar::base64decode(str_encode);
    std::cout << "abc -> " << str_encode << " -> " << str_decode << std::endl;

    std::string str_encode2 = sylar::base64encode("ab");
    std::string str_decode2 = sylar::base64decode(str_encode2);
    std::cout << "ab -> " << str_encode2 << " -> " << str_decode2 << std::endl;

    std::string str_encode3 = sylar::base64encode("a");
    std::string str_decode3 = sylar::base64decode(str_encode3);
    std::cout << "a -> " << str_encode3 << " -> " << str_decode3 << std::endl;
}

int main(int argc, char const *argv[]) {
    test_base64encode();
    return 0;
}
