/**
 * Created by lisongqian on 2022/3/31.
 * 跨平台完成 2022-4-7 15:40:38
 */

#ifndef NETDISK_LOCKER_H
#define NETDISK_LOCKER_H

#include <exception>

#ifdef UNIX
#include <pthread.h>
#include <semaphore.h>

class sem
{
public:
    sem()
    {
        if (sem_init(&m_lock, 0, 0) != 0)
        {
            throw std::exception();
        }
    }
    sem(int num)
    {
        if (sem_init(&m_lock, 0, num) != 0)
        {
            throw std::exception();
        }
    }
    ~sem()
    {
        sem_destroy(&m_lock);
    }
    bool wait()
    {
        return sem_wait(&m_lock) == 0;
    }
    bool signal()
    {
        return sem_post(&m_lock) == 0;
    }

private:
    sem_t m_lock;
};

class locker
{
public:
    locker()
    {
        if (pthread_mutex_init(&m_mutex, NULL) != 0)
        {
            throw std::exception();
        }
    }
    ~locker()
    {
        pthread_mutex_destroy(&m_mutex);
    }
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    pthread_mutex_t *get()
    {
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;
};
class cond
{
public:
    cond()
    {
        if (pthread_cond_init(&m_cond, NULL) != 0)
        {
            //pthread_mutex_destroy(&m_mutex);
            throw std::exception();
        }
    }
    ~cond()
    {
        pthread_cond_destroy(&m_cond);
    }
    bool wait(pthread_mutex_t *m_mutex)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_wait(&m_cond, m_mutex);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *m_mutex, struct timespec t)
    {
        int ret = 0;
        //pthread_mutex_lock(&m_mutex);
        ret = pthread_cond_timedwait(&m_cond, m_mutex, &t);
        //pthread_mutex_unlock(&m_mutex);
        return ret == 0;
    }
    bool signal()
    {
        return pthread_cond_signal(&m_cond) == 0;
    }
    bool broadcast()
    {
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    //static pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};
#else

#include <windows.h>

class sem {
public:
    sem() {
        m_sem = CreateSemaphore(nullptr, 0, 10, nullptr);
    }

    sem(int num) {
        m_sem = CreateSemaphore(nullptr, num, num, nullptr);// 第2个参数是初始值、第3个是最大值
    }

    ~sem() {
        CloseHandle(m_sem);
    }
    void wait()
    {
        WaitForSingleObject(m_sem, INFINITE);
    }
    void signal()
    {
        ReleaseSemaphore(m_sem, 1, nullptr);
    }

private:
    HANDLE m_sem;
};

class locker {
public:
    locker() {
        m_lock = CreateMutex(nullptr, false, nullptr);
    }

    ~locker() {
        CloseHandle(m_lock);
    }

    bool lock() {
        WaitForSingleObject(m_lock, INFINITE);
        return true;
    }

    bool unlock() {
        ReleaseMutex(m_lock);
        return true;
    }

private:
    HANDLE m_lock;
};

#endif

#endif //NETDISK_LOCKER_H
