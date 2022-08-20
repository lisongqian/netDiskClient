//
// Created by lisongqian on 2022/4/9.
//

#ifndef NETDISK_HTTPREQUEST_H
#define NETDISK_HTTPREQUEST_H

#include <iostream>
#include <QFileInfo>
#ifdef UNIX
#include <pthread.h>
#else

//#include <WinSock2.h>
//#pragma comment(lib, "ws2_32.lib")  //加载 ws2_32.dll

#include <winsock.h>
#pragma comment(lib, "wsock32.lib")

#endif

using std::map;
using std::string;
using std::vector;

class HTTPRequest {
public:
    HTTPRequest(std::string ip, int port);

    ~HTTPRequest();

    bool init();

    bool close_socket();

    bool get(const string &url, const map<string, string> &data, string &response) const;

    bool post(const string &url, const map<string, string> &data, string &response) const;

    bool post(const string &url, const map<string, string> &data, const map<string, string> &headers, string &response) const;

    bool sendFile(const string &url, const vector<std::shared_ptr<QFileInfo>> &files, int current_dir,
                  map<string, string> &headers,
                  string &response) const;

    bool downloadFile(const string &url, const map<string, string> &data, const map<string, string> &headers,
                      const string& filename) const;

private:
    string m_ip;
    short m_port;
    bool m_open;
    SOCKET m_socket;
};


#endif //NETDISK_HTTPREQUEST_H
