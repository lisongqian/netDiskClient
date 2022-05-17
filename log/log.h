/**
 * 通用组件————日志
 */

#ifndef NETDISK_LOG_H
#define NETDISK_LOG_H

#include <iostream>

#ifdef UNIX
#include <pthread.h>
#else

#include <windows.h>

#endif

#include <queue>
#include "common.h"
#include "../lock/locker.h"

using std::string;
using std::queue;

class Log {
public:
    static Log *instance() {
        //C++0x后，静态变量线程安全。Warning：C++11前需加锁！！！
        static Log _instance;
        return &_instance;
    }

    bool Init(int log_buff_size);

    void WriteLogToQueue(int level, const char *format, ...);

#ifdef UNIX
    static void *FlushLogThreadU(void *args)
    {
        Log::instance()->AsyncWriteLog();
    }
#else

    static DWORD WINAPI FlushLogThreadW(LPVOID args) {
        Log::instance()->AsyncWriteLog();
        return 1;
    }

#endif
private:
    Log() = default;

    virtual ~Log();

    void *AsyncWriteLog();

private:
    FILE *m_fp;             // 日志文件句柄
    int m_log_buf_size;     // 日志缓冲区大小
    queue<char *> m_queue;   // 日志输出队列
    sem m_queue_sem;        // 日志队列信号量
    locker m_queue_lock;    // 日志队列互斥锁
#ifdef UNIX
    pthread_t m_pthread;    // 日志写入线程
#else
    HANDLE m_pthread;       // 新线程
#endif
};

#define LOG_DEBUG(format, ...)  {Log::instance()->WriteLogToQueue(E_DEBUG, format, ##__VA_ARGS__);}
#define LOG_INFO(format, ...) {Log::instance()->WriteLogToQueue(E_INFO, format, ##__VA_ARGS__);}
#define LOG_WARN(format, ...) {Log::instance()->WriteLogToQueue(E_WARN, format, ##__VA_ARGS__);}
#define LOG_ERROR(format, ...) {Log::instance()->WriteLogToQueue(E_ERROR, format, ##__VA_ARGS__);}


#endif //NETDISK_LOG_H
