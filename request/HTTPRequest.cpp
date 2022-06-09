//
// Created by lisongqian on 2022/4/9.
//

#include <cstring>
#include <utility>
#include "HTTPRequest.h"
#include "log/log.h"
#include "common.h"

HTTPRequest::HTTPRequest(string ip, int port) : m_ip(std::move(ip)), m_port(port), m_open(false), m_socket(0) {
}

bool HTTPRequest::init() {
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA w_data;
    if (WSAStartup(sockVersion, &w_data) != 0) {
        m_open = false;
    }
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        LOG_ERROR("invalid socket!");
        m_open = false;
        return false;
    }
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(m_port);
    serAddr.sin_addr.S_un.S_addr = inet_addr(m_ip.c_str());
    int timeout = 3000; //3s
    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout));
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
    if (connect(m_socket, (sockaddr *) &serAddr, sizeof(serAddr)) == SOCKET_ERROR) {  //连接失败
        m_open = false;
        LOG_ERROR("connect error !");
        closesocket(m_socket);
        return false;
    }
    m_open = true;
    return true;
}

bool HTTPRequest::get(const string &url, const map<string, string> &data, string &response) const {
    if (!m_open) {
        return false;
    }
    string send_buff = Map2String(data);
    const char *sendData;
    sendData = send_buff.c_str();   //string转const char*
    send(m_socket, sendData, strlen(sendData), 0);
    //int send(int s, const void * msg, int len, unsigned int flags)
    char recData[2048];
    memset(recData, '\0', 2048);
    int ret = recv(m_socket, recData, 2048, 0);
    if (ret > 0) {
        recData[ret] = 0x00;
        LOG_INFO(recData)
        response = recData;
    }
    return true;
}


bool HTTPRequest::post(const string &url, const map<string, string> &data, string &response) const {
    if (!m_open) {
        return false;
    }
    char send_data[100];
    memset(send_data, '\0', 100);
    int data_len = 0;
    for (const auto &it: data) {
        data_len += snprintf(send_data + data_len, 100 - data_len, "%s=%s&", it.first.c_str(),
                             it.second.c_str());
    }
    send_data[strlen(send_data) - 1] = '\0';
    char send_buff[2048];
    memset(send_buff, '\0', 2048);
    int n = snprintf(send_buff, 2048, "POST\t%s\tHTTP/1.1\r\n", url.c_str());
    n += snprintf(send_buff + n, 2048 - n, "Connection:keep-alive\r\n");
    n += snprintf(send_buff + n, 2048 - n, "Content-length:%d\r\n", data_len);
    snprintf(send_buff + n, 2048 - n, "\r\n%s", send_data);
//    LOG_INFO("send:%s", send_buff);
    send(m_socket, send_buff, strlen(send_buff) + 1, 0);
    //int send(int s, const void * msg, int len, unsigned int flags)
    char recData[2048];
    memset(recData, '\0', 2048);
    int ret = recv(m_socket, recData, 2048, 0);
    if (ret > 0) {
        recData[ret] = 0x00;
        string tmp = recData;
        int pos = tmp.rfind('\n');
        if (pos > 0) {
            response = tmp.substr(pos + 1);
            LOG_INFO("response:%s", response.c_str())
            return true;
        }
    }
    return false;
}

bool HTTPRequest::close_socket() {
    closesocket(m_socket);
    WSACleanup();
    m_open = false;
    return true;
}

HTTPRequest::~HTTPRequest() {
    if (m_open) {
        close_socket();
    }
}

