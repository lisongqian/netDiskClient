//
// Created by lisongqian on 2022/4/9.
//

#ifndef NETDISK_HTTPREQUEST_H
#define NETDISK_HTTPREQUEST_H

#include <iostream>
#include <map>
using std::map;
using std::string;

class HTTPRequest {
public:
    HTTPRequest(const std::string& ip, int port);
    ~HTTPRequest() =default;
    bool Get(std::string url,map<string,string> data);
    bool Post(std::string url,map<string,string> data);
private:
    std::string m_ip;
    int m_port;
};


#endif //NETDISK_HTTPREQUEST_H
