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
    bool Get(std::string url,map<string,string> data,string &response);
    bool Post(std::string url,map<string,string> data,string &response);
private:
    std::string m_ip;
    int m_port;
};


#endif //NETDISK_HTTPREQUEST_H
