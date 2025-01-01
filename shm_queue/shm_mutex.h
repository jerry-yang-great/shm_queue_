#pragma once

//#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <pthread.h>

//比较简单的跨进程同步方式：XSI信号量和POSIX信号量、文件锁
class SHMMutex {
public:
    SHMMutex();
    ~SHMMutex();

    // ptr是一个位于SHM内存区的指针
    void Init(pthread_mutex_t* ptr);   
    void Release();
 
    void lock();
    void unlock();

private:
    pthread_mutex_t* ptr_{nullptr};
};