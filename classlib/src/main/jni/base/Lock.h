//
// Created by jarlene on 2015/9/21.
//

#ifndef COMMONDEMO_LOCK_H
#define COMMONDEMO_LOCK_H

#include <pthread.h>

class CLock {
public:
    CLock() {
        pthread_mutexattr_t ma;
        pthread_mutexattr_init(&ma);
        pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&m_mutex,&ma);
    }
    ~CLock() {
        pthread_mutex_destroy(&m_mutex);
    }

    inline void Lock() {
        pthread_mutex_lock(&m_mutex);
    }

    inline void Unlock() {
        pthread_mutex_unlock(&m_mutex);
    }
protected:
    pthread_mutex_t m_mutex;
};

// 自动线程锁对象
class CAutoLock {
public:
    CAutoLock(CLock &lock) : m_pLock(&lock){
            m_pLock->Lock();
    }
    ~CAutoLock() {
        m_pLock->Unlock();
    }
protected:
    CLock * m_pLock;
};

#endif //COMMONDEMO_LOCK_H
