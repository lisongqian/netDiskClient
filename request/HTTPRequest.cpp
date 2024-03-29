//
// Created by lisongqian on 2022/4/9.
//

#include <cstring>
//#include <utility>
#include <map>
#include <QCryptographicHash>
#include <direct.h>
#include "HTTPRequest.h"
#include "log/log.h"
#include "common.h"
#include "config.h"

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
    int timeout = 3 * 1000;
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
    int n = snprintf(send_buff, 2048, "GET\t%s?%s\tHTTP/1.1\r\n", url.c_str(), send_data);
    snprintf(send_buff + n, 2048 - n, "Connection:keep-alive\r\n");
//    n += snprintf(send_buff + n, 2048 - n, "Content-length:%d\r\n", data_len);
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


bool HTTPRequest::post(const string &url, const map<string, string> &data, string &response) const {
    const map<string, string> headers;
    return post(url, data, headers, response);
}

bool HTTPRequest::post(const string &url, const map<string, string> &data, const map<string, string> &headers,
                       string &response) const {
    if (!m_open) {
        return false;
    }
    int send_data_len = 256;// 默认缓冲区长度
    char *send_data = new char[send_data_len];
    memset(send_data, '\0', send_data_len);
    int data_length = 0;// 实际字符串长度
    /*json数据*/
    for (const auto &it: data) {
        data_length += snprintf(send_data + data_length, send_data_len - data_length, "%s=%s&",
                                it.first.c_str(),
                                it.second.c_str());
    }
    if (data_length > 0)
        send_data[data_length - 1] = '\0';
    unsigned int send_buff_size = 2048 + data_length; // 发送缓冲区长度
    char *send_buff = new char[send_buff_size];
    memset(send_buff, '\0', *send_buff);
    int n = snprintf(send_buff, send_buff_size, "POST\t%s\tHTTP/1.1\r\n", url.c_str());
    n += snprintf(send_buff + n, send_buff_size - n, "Connection:keep-alive\r\n");
    n += snprintf(send_buff + n, send_buff_size - n, "Content-length:%d\r\n", strlen(send_data));
    for (auto &item: headers) {
        n += snprintf(send_buff + n, send_buff_size - n, "%s:%s\r\n", item.first.c_str(), item.second.c_str());
    }
    n += snprintf(send_buff + n, send_buff_size - n, "\r\n%s", send_data);
//    LOG_DEBUG("send:%s", send_buff);
    send(m_socket, send_buff, n, 0);
    //int send(int s, const void * msg, int len, unsigned int flags)
    delete[] send_data;
    delete[] send_buff;
    char recData[2048];
    memset(recData, '\0', 2048);
    int ret = recv(m_socket, recData, 2048, 0);
    if (ret > 0) {
        recData[ret] = 0x00;
        string tmp = recData;
        int pos = tmp.rfind('\n');
        if (pos > 0) {
            response = tmp.substr(pos + 1);
//            LOG_DEBUG("response:%s", response.c_str())
            return true;
        }
    }
    return false;
}

bool HTTPRequest::sendFile(const string &url, const std::vector<std::shared_ptr<QFileInfo>> &files,
                           int current_dir,
                           map<string, string> &headers,
                           string &response) const {
    if (!m_open) {
        return false;
    }

    char mix_str[64];
    memset(mix_str, '\0', sizeof(mix_str));
    sscanf(headers["Content-Type"].c_str(), "multipart/form-data; boundary=%s", mix_str);

    for (auto &file: files) {
        FILE *fp = fopen(file->filePath().toLocal8Bit().toStdString().c_str(), "rb+");
        if (fp == nullptr) {
            LOG_ERROR("%s open error.", file->filePath().toLocal8Bit().toStdString().c_str());
            continue;
        }
        fseek(fp, 0L, SEEK_SET);
//        fseek(fp, 0L, SEEK_END);
//        int file_size = ftell(fp);
        QCryptographicHash ch(QCryptographicHash::Md5);
        char buf[4096];
        size_t size;
        int file_size = 0;
        while ((size = fread(buf, 1, 4096, fp)) != 0) {
            ch.addData(buf, static_cast<int>(size));
            file_size += static_cast<int>(size);
        }
        LOG_DEBUG("md5:%s", ch.result().toHex().data());

        char file_head_data[2048];
        memset(file_head_data, '\0', sizeof(file_head_data));
        int file_head_data_len = snprintf(file_head_data, sizeof(file_head_data), "----%s\r\n", mix_str);
        file_head_data_len += snprintf(file_head_data + file_head_data_len, sizeof(file_head_data) - file_head_data_len,
                                       "Content-Disposition: form-data; parent=\"%d\"; name=\"%s\"; filename=\"%s\"; size=%d\r\n\r\n",
                                       current_dir, file->fileName().toStdString().c_str(),
                                       ch.result().toHex().toStdString().c_str(), file_size);
        /**
         * http请求
         */
        unsigned int send_buff_size = 2048; // 发送缓冲区长度
        char send_buff[2048];
        memset(send_buff, '\0', send_buff_size);
        int n = snprintf(send_buff, send_buff_size, "POST\t%s\tHTTP/1.1\r\n", url.c_str());
        n += snprintf(send_buff + n, send_buff_size - n, "Connection:keep-alive\r\n");
        n += snprintf(send_buff + n, send_buff_size - n, "Content-length:%d\r\n",
                      file_size + file_head_data_len + strlen(mix_str) + 8);
        for (auto &header: headers) {
            n += snprintf(send_buff + n, send_buff_size - n, "%s:%s\r\n", header.first.c_str(), header.second.c_str());
        }
        n += snprintf(send_buff + n, send_buff_size - n, "\r\n");
        send(m_socket, send_buff, n, 0);// 第一次发送——发送请求头
        Sleep(2);
        //int send(int s, const void * msg, int len, unsigned int flags)
        send(m_socket, file_head_data, file_head_data_len, 0);  // 第二次发送——发送请求体的文件头
        rewind(fp);
        while ((size = fread(buf, 1, 4096, fp)) != 0) {
            send(m_socket, buf, static_cast<int>(size), 0);// 第三次发送——发送请求体的文件数据
        }
        int total_size = n + file_size + file_head_data_len;
        int file_end_data_len = snprintf(file_head_data, sizeof(file_head_data), "\r\n----%s--", mix_str);
        total_size += file_end_data_len;
        send(m_socket, file_head_data, file_end_data_len, 0);  // 第四次发送请求体的文件结束符
//        LOG_DEBUG("file_end:%s", file_head_data)
        LOG_DEBUG("parent:%d,file_size:%d", current_dir, file_size);
        fclose(fp);
    }

    /**
     * 接收数据
     */
    char recData[2048];
    memset(recData, '\0', 2048);
    int ret = recv(m_socket, recData, 2048, 0);
    if (ret > 0) {
        recData[ret] = 0x00;
        string tmp = recData;
        int pos = tmp.rfind('\n');
        if (pos > 0) {
            response = tmp.substr(pos + 1);
            return true;
        }
    }
    return false;
}


bool HTTPRequest::downloadFile(const string &url, const map<string, string> &data, const map<string, string> &headers,
                               const string &filename) const {
    if (!m_open) {
        return false;
    }
    int send_data_len = 256;// 默认缓冲区长度
    char *send_data = new char[send_data_len];
    memset(send_data, '\0', send_data_len);
    int data_length = 0;// 实际字符串长度
    /*json数据*/
    for (const auto &it: data) {
        data_length += snprintf(send_data + data_length, send_data_len - data_length, "%s=%s&",
                                it.first.c_str(),
                                it.second.c_str());
    }
    if (data_length > 0)
        send_data[data_length - 1] = '\0';
    unsigned int send_buff_size = 2048 + data_length; // 发送缓冲区长度
    char *send_buff = new char[send_buff_size];
    memset(send_buff, '\0', *send_buff);
    int n = snprintf(send_buff, send_buff_size, "POST\t%s\tHTTP/1.1\r\n", url.c_str());
    n += snprintf(send_buff + n, send_buff_size - n, "Connection:keep-alive\r\n");
    n += snprintf(send_buff + n, send_buff_size - n, "Content-length:%d\r\n", strlen(send_data));
    for (auto &item: headers) {
        n += snprintf(send_buff + n, send_buff_size - n, "%s:%s\r\n", item.first.c_str(), item.second.c_str());
    }
    n += snprintf(send_buff + n, send_buff_size - n, "\r\n%s", send_data);
//    LOG_DEBUG("send:%s", send_buff);
    send(m_socket, send_buff, n, 0);
    //int send(int s, const void * msg, int len, unsigned int flags)
    delete[] send_data;
    delete[] send_buff;
    extern Config g_config;
    string file_path = g_config.download_path;
    if (access(file_path.c_str(), 0) == -1) {
        mkdir(file_path.c_str());
    }
    file_path += filename;
    char *recData = new char[4096];
    memset(recData, '\0', 4096);
    FILE *fp = fopen(file_path.c_str(), "wb");
    int ret;
    do {
        ret = recv(m_socket, recData, 4096, 0);
        if (ret >= 0) {
            fwrite(recData, 1, ret, fp);
        }
        else {
            LOG_ERROR("recv failed: %d", WSAGetLastError());
        }
    }
    while (ret > 0);
    fclose(fp);
    return true;
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

