//
// Created by lisongqian on 2022/4/7.
//

#ifndef NETDISK_CONFIG_H
#define NETDISK_CONFIG_H

#include <getopt.h>
#include <cstdlib>
#include <cstring>

using std::string;

class Config {
public:
    Config() : port(23450), buff_size(2048),  login_cache_path("./tmp/login.cache"),token(""),ip("127.0.0.1") {}
//    ip = "192.168.229.129";
    void ParseArg(int argc, char *argv[]) {
        int opt;
        const char *str = "p:b:d:";
        while ((opt = getopt(argc, argv, str)) != -1) {
            switch (opt) {
                case 'p': {
                    port = atoi(optarg);//char* to int
                    break;
                }
                case 'b': {
                    buff_size = atoi(optarg);
                    break;
                }

                default: {
                    break;
                }
            }
        }
    }

    int port;
    int buff_size;
    string login_cache_path;
    string token;
    string ip;
};

#endif //NETDISK_CONFIG_H
