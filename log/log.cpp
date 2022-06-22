/**
 * 日志类实现
 */

#include <cstring>
#include <cstdarg>
#include <sys/time.h>
#include "log.h"

#ifndef LOG_LEVEL
#define LOG_LEVEL alertLevel::E_INFO
#endif

Log::~Log() {
    if (m_fp != nullptr) {
        fflush(m_fp);
        fclose(m_fp);
        m_fp = nullptr;
    }
#ifdef UNIX

#else
    CloseHandle(m_pthread);
#endif
}

bool Log::Init(int log_buff_size) {
    m_log_buf_size = log_buff_size;

    char logFileName[256] = {0};
    time_t now_t = time(nullptr);
    struct tm *now_tm = localtime(&now_t);
    snprintf(logFileName, 255, "./tmp/netDisk_%d_%02d_%02d.log", now_tm->tm_year + 1900, now_tm->tm_mon + 1,
             now_tm->tm_mday);

    if (m_fp == nullptr) {
        m_fp = fopen(logFileName, "a");
    }
#ifdef UNIX
    pthread_create(&m_pthread, nullptr, FlushLogThreadU, nullptr);
#else
    m_pthread = CreateThread(nullptr, 0, FlushLogThreadW, nullptr, 0, nullptr);
#endif
    return true;
}

void Log::WriteLogToQueue(alertLevel level, const char *format, ...) {
    if (level > LOG_LEVEL) {
        return ;
    }
    char s[16] = {0};
    switch (level) {
        case alertLevel::E_DEBUG: {
            strcpy(s, "[debug]:");
            break;
        }
        case alertLevel::E_INFO: {
            strcpy(s, "[info]:");
            break;
        }
        case alertLevel::E_WARN: {
            strcpy(s, "[warn]:");
            break;
        }
        case alertLevel::E_ERROR: {
            strcpy(s, "[error]:");
            break;
        }
        default:
            break;
    }
    va_list valst;
    va_start(valst, format);
    char *str_buff = new char[m_log_buf_size];
    memset(str_buff, '\0', m_log_buf_size);
    //写入的具体时间内容格式
    struct timeval now_u = {0, 0}; // 获取微秒
    gettimeofday(&now_u, nullptr);
    time_t now_t = now_u.tv_sec;
    struct tm *now_tm = localtime(&now_t);
    int n = snprintf(str_buff, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday,
                     now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec, now_u.tv_usec, s);

    int m = vsnprintf(str_buff + n, m_log_buf_size - n - 2, format, valst);
    if (n + m < m_log_buf_size)
        str_buff[n + m] = '\n';
    else {
        str_buff[m_log_buf_size - 2] = '\n';
    }
    m_queue_lock.lock();
    m_queue.push(str_buff);
    m_queue_lock.unlock();
    m_queue_sem.signal();
    va_end(valst);
}

void *Log::AsyncWriteLog() {
    while (true) {
        m_queue_sem.wait();
        m_queue_lock.lock();
        if (!m_queue.empty()) {
            char *tmp = m_queue.front();
            std::cout << tmp;
            fputs(tmp, m_fp);
            delete[] tmp;
            m_queue.pop();
        }
        fflush(m_fp);
        m_queue_lock.unlock();
    }
}
