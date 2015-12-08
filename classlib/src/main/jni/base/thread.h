//
// Created by Jarlene on 2015/9/21.
//

#ifndef COMMONDEMO_THREAD_H
#define COMMONDEMO_THREAD_H

#include "thread.h"
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include "Lock.h"


#define THREAD_HANDLE pthread_t
#define THREAD_PROC void *
#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif


#define THREAD_MAX_PRIORITY 100
#define THREAD_MIN_PRIORITY 0
#define THREAD_NORMAL_PRIORITY 10

#ifdef SleepMS
#undef SleepMS
#endif
#define SleepMS(uMillisecond) usleep(uMillisecond*1000)

class CThread {
public:
    CThread(const char* pName = NULL);
    virtual ~CThread();
public:
    inline uint32_t GetThreadID(){return m_uThreadID;};	///< 返回线程号

    bool IsActivate();///< 线程是否正在运行

    bool IsSelfThread();

    virtual bool Start();	///< 启动线程
    virtual bool Terminate(uint32_t uTimeout = INFINITE);		///< 结束线程，超时强制结束，该函数只能由外部线程调用
protected:
    bool WaitToExit(uint32_t uTimeout = 0);
    bool WaitUntilExit();
    uint32_t GetStatus();
    const char* GetName();
    void SetName(const char* pName);
    virtual void Run(){}; ///< 线程主函数,如果是循环，则在循环中调用WaitToExit函数，返回ture时退出
private:
    static THREAD_PROC ThreadProc(void* pParam);
    static uint32_t GetCurrentThreadId();
private:
    THREAD_HANDLE m_hThread;
    uint32_t m_uThreadID;
    uint32_t m_uThreadStatus;
    CLock m_lockStatus;
    const char* pThreadNameM;
};


#endif //COMMONDEMO_THREAD_H
