#include "mutex.h"

#include <sys/file.h>

const std::string Mutex::file_name_("/dev/shm/shm.lock");

Mutex::Mutex() {
    fd_ = open(file_name_.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd_ == -1) {
        perror("open");
        exit(1);
    }
}

Mutex::~Mutex() {
    if (fd_ != -1) {
        close(fd_);
    }
    fd_ = -1;
}

void Mutex::lock() {
    if (flock(fd_, LOCK_EX) == -1) { // 加锁后进程崩溃，是可以自动解锁的
        perror("flock LOCK_EX");
    }
}

void Mutex::unlock() {
    if (flock(fd_, LOCK_UN) == -1) {
        perror("flock LOCK_UN");
    }
}