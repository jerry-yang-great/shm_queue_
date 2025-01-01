#include "shm_mutex.h"

#include <sys/file.h>

#include <iostream>

SHMMutex::SHMMutex() {
}

SHMMutex::~SHMMutex() {
    Release();
}

void SHMMutex::Init(pthread_mutex_t* ptr) {
    ptr_ = ptr;
    if (!ptr_) {
        std::cerr << "ptr is nullptr" << std::endl;
        return;
    }

    // 初始化健壮互斥锁
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST); // 加锁后崩溃，可以自动恢复解锁.

    pthread_mutex_init(ptr_, &attr);

    // 销毁属性
    pthread_mutexattr_destroy(&attr);
}

void SHMMutex::Release() {
    //  由于mutex在共享内存上，这里不能销毁mutex，否则会导致其他进程操作一个已经被销毁的mutex
    // if (ptr_) {
    //    pthread_mutex_destroy(ptr_);
    // }
    ptr_ = nullptr;
}

void SHMMutex::lock() {
    if (!ptr_) {
        std::cerr << "ptr is nullptr" << std::endl;
        return;
    }

    // 尝试获取锁
    int lock_status = pthread_mutex_lock(ptr_);
    if (lock_status == EOWNERDEAD) {
        // 锁的拥有者崩溃了，需要修复锁
        std::cout << "Previous process holding the lock has \
             crashed. Recovering the lock..." << std::endl;
        pthread_mutex_consistent(ptr_);
    }
}

void SHMMutex::unlock() {
    if (!ptr_) {
        std::cerr << "ptr is nullptr" << std::endl;
        return;
    }

    pthread_mutex_unlock(ptr_);
}