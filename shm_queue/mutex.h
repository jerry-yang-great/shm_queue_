#pragma once

//#include <stdio.h>
//#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string>

//比较简单的跨进程同步方式：XSI信号量和POSIX信号量、文件锁(flock函数和fcntl函数)
class Mutex {
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    int fd_{-1};
    //flock lock_;

    static const std::string file_name_;
};