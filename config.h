//
// Created by lisongqian on 2022/4/7.
//

#ifndef NETDISK_CONFIG_H
#define NETDISK_CONFIG_H

#include <getopt.h>
#include <cstdlib>

class Config{
public:
    Config():port(23450),buff_size(2048),database_connect_num(8){}
    void ParseArg(int argc,char* argv[]){
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
                case 'd': {
                    database_connect_num = atoi(optarg);
                }
                default: {
                    break;
                }
            }
        }
    }
    int port;
    int buff_size;
    int database_connect_num;
};
#endif //NETDISK_CONFIG_H
