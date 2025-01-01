#include "shm_manager.h"

#include <fcntl.h>  // For O_* constants
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>
//#include <mutex>
#include <cstring>

const std::string ShmManager::shm_name_("/shm_queue");

ShmManager::ShmManager() {
}

ShmManager::~ShmManager() {
    Release();
}

bool ShmManager::Init(uint64_t shm_size, bool is_manager) {
    shm_size_ = shm_size;
    is_manager_ = is_manager;

    int oflag = O_RDWR; // | O_EXCL;
    if (is_manager_) {
        // manager进程需要O_CREAT标记创建SHM资源
        oflag |= O_CREAT;
        // manager进程需要删除历史残留文件，normal进程不需要
        shm_unlink(shm_name_.c_str());
    }

    // 1. 创建共享内存对象
    shm_fd_ = shm_open(shm_name_.c_str(), oflag, 0666);
    if (shm_fd_ == -1) {
        std::cerr << "shm_open failed:" << strerror(errno) << std::endl;
        return false;
    }

    if (is_manager_) {
        // 2. 设置共享内存大小，normal进程不需要
        if (ftruncate(shm_fd_, shm_size_) == -1) {
            std::cerr << "ftruncate failed:" << strerror(errno) << std::endl;
            return false;
        }
    }

    // 3. 将共享内存映射到当前进程地址空间
    start_ptr_ = mmap(0, shm_size_, PROT_WRITE|PROT_READ, MAP_SHARED, shm_fd_, 0);
    if (start_ptr_ == MAP_FAILED) {
        std::cerr << "mmap failed:" << strerror(errno) << std::endl;
        start_ptr_ = nullptr;
        return false;
    }

    return true;
}

void ShmManager::Release() {
    // 1. ​解除映射
    if (start_ptr_ != nullptr) {
        munmap(start_ptr_, shm_size_);
        start_ptr_ = nullptr;
    }
    // 2. 关闭共享内存对象
    if (shm_fd_ != -1) {
        close(shm_fd_);
        shm_fd_ = -1;
    }
    // 3. 删除位于/dev/shm下的shm文件,normal进程不需要
    if (is_manager_) {
        shm_unlink(shm_name_.c_str());
    }

    shm_size_ = 0;
}

void* ShmManager::GetMemory(uint64_t& mem_size) {
    mem_size = shm_size_;
    return start_ptr_;
}
