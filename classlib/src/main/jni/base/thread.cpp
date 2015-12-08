//
// 线程的封装，可以知道线程的状态，线程名，线程号等特点。
// Created by Jarlene on 2015/9/21.
//

#include "thread.h"
#include "log.h"
#include "Lock.h"
#include <sys/syscall.h>
#include <sys/types.h>

#define LOG_TAG "Native thread"

enum THREAD_STATUS {
    THREAD_STATUS_START_PENDING,
    THREAD_STATUS_START,
    THREAD_STATUS_STOP_PENDING,
    THREAD_STATUS_STOP,
};

CThread::CThread(const char* pName)
        : m_uThreadStatus(THREAD_STATUS_STOP)
        , m_hThread((THREAD_HANDLE) NULL)
        , m_uThreadID(0)
        , pThreadNameM(pName) {

}

CThread::~CThread() {
    Terminate(1000);
}


bool CThread::IsActivate() {
    CAutoLock lock(m_lockStatus);
    if ( m_uThreadStatus == THREAD_STATUS_START ) {
        return true;
    }
    return false;
}


bool CThread::Start() {
    CAutoLock lock(m_lockStatus);
    if ( m_uThreadStatus != THREAD_STATUS_STOP) {
        return true;
    } else {
        m_uThreadStatus = THREAD_STATUS_START_PENDING;
        m_uThreadID = 0;
        m_hThread = (THREAD_HANDLE) NULL;
    }
    if(0 == pthread_create(&m_hThread,NULL,ThreadProc,this)) {
        CAutoLock lock(m_lockStatus);
        pthread_detach(m_hThread);
        m_uThreadID = (uintptr_t)m_hThread;
        return true;
    } else {
        CAutoLock lock(m_lockStatus);
        m_uThreadStatus = THREAD_STATUS_STOP;
        return false;
    }
    return true;
}

bool CThread::Terminate( uint32_t uTimeout /*= INFINITE*/ ) {
    CAutoLock lock(m_lockStatus);
    if (m_uThreadStatus == THREAD_STATUS_START_PENDING
        || m_uThreadStatus == THREAD_STATUS_START) {
        m_uThreadStatus = THREAD_STATUS_STOP_PENDING;
    } else if ( m_uThreadStatus == THREAD_STATUS_STOP ) {
        return true;
    }
    int loop_count = uTimeout/10;
    bool bForce = true;
    do {
        CAutoLock lock(m_lockStatus);
        if ( m_uThreadStatus == THREAD_STATUS_STOP) {
            bForce = false;
            break;
        }
        if (loop_count > 0) {
            SleepMS(10);
            loop_count --;
        }
    } while (loop_count > 0);

    if ( bForce == true) {
        CAutoLock lock(m_lockStatus);
    }
    return !bForce;
}



bool CThread::WaitToExit( uint32_t uTimeout /*= 0*/ ) {
    int32_t loop_count = uTimeout/10;
    bool bExit = false;
    do {
        CAutoLock lock(m_lockStatus);
        if ( m_uThreadStatus == THREAD_STATUS_STOP_PENDING) {
            bExit = true;
            break;
        }
        if (loop_count > 0) {
            SleepMS(10);
        }
        loop_count --;
    } while (loop_count >= 0);
    return bExit;
}


THREAD_PROC CThread::ThreadProc( void* pParam ) {
    CThread *pThis = (CThread *) pParam;
    pThis->m_lockStatus.Lock();
    pThis->m_uThreadStatus = THREAD_STATUS_START;
    pThis->m_lockStatus.Unlock();
    pThis->m_uThreadID = pThis->GetCurrentThreadId();
    pThis->Run();
    pThis->m_lockStatus.Lock();
    pThis->m_uThreadStatus = THREAD_STATUS_STOP;
    pThis->m_hThread = (THREAD_HANDLE) NULL;
    pThis->m_lockStatus.Unlock();
    return 0;
}


uint32_t CThread::GetCurrentThreadId() {
    uint32_t uTreadId = 0;
    uTreadId = syscall(__NR_gettid);
    return uTreadId;
}


bool CThread::IsSelfThread() {
    return m_uThreadID == this->GetCurrentThreadId();
}

bool CThread::WaitUntilExit() {
    int count = 0x0;
    while(1) {
        CAutoLock lock(m_lockStatus);
        if(THREAD_STATUS_STOP == m_uThreadStatus) {
            break;
        }
        SleepMS(10);
        count++;
    }
    LOGW("m_uThreadID %d %s WaitUntilExit end",(uint32_t)pthread_self(), GetName());
    return true;
}

uint32_t CThread::GetStatus() {
    CAutoLock lock(m_lockStatus);
    return m_uThreadStatus;
}

const char* CThread::GetName() {
    return ((pThreadNameM) ? pThreadNameM : "unknown");
}

void CThread::SetName(const char* pName) {
    CAutoLock lock(m_lockStatus);
    pThreadNameM = pName;
}