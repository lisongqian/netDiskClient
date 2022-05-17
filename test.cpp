//
// Created by lisongqian on 2022/5/16.
//
#include <iostream>
#include <fstream>
#include <direct.h>

int main() {
    if (0 != access("tmp", 0)) {
        mkdir("./tmp");
    }
    std::string username = "lisongqian", password = "123123";
    std::ofstream out("./tmp/login.cache.tmp");
    if (out.is_open()) {
        out << username << std::endl;
        out << password << std::endl;
        std::cout << "success" << std::endl;
        out.close();
    }
    else {
        std::cout << "failed" << std::endl;
    }
    return 0;
}
