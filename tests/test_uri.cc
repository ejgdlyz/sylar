#include "sylar/uri.h"
#include <iostream>

void test_uri() {
    // sylar::Uri::ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri?id=100&name=sylar#frg");
    sylar::Uri::ptr uri = sylar::Uri::Create("http://admin@www.sylar.top:8080/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    // sylar::Uri::ptr uri = sylar::Uri::Create("http://www.sylar.top/test/uri");

    std::cout << uri->toString() << std::endl;

    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
}   

int main(int argc, char const *argv[]) {
    test_uri();
    return 0;
}

