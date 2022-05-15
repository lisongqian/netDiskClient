//
// Created by lisongqian on 2022/4/9.
//

#include "HTTPRequest.h"

HTTPRequest::HTTPRequest(const std::string &ip, int port) : m_ip(ip), m_port(port) {

}

bool HTTPRequest::Get(std::string url, map<string, string> data,std::string &response) {

    return true;
}

bool HTTPRequest::Post(std::string url, map<string, string> data,std::string &response) {
    return true;
}

