//
// Created by lisongqian on 2022/4/9.
//

#ifndef NETDISK_HTTPREQUEST_H
#define NETDISK_HTTPREQUEST_H

#include <iostream>
#include <map>

#ifdef UNIX
#include <pthread.h>
#else

#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll
#endif

using std::map;
using std::string;

class HTTPRequest {
public:
    HTTPRequest(std::string  ip, int port);
    ~HTTPRequest();
    bool init();
    bool close_socket();
    bool get(string url, const map<string,string>& data, string &response);
    bool post(string url, map<string,string> data, string &response);
private:
    string m_ip;
    int m_port;
    bool m_open;
    SOCKET m_socket;
};


#endif //NETDISK_HTTPREQUEST_H
