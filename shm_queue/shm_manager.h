#pragma once

#include <stdint.h>
#include <string>

class ShmManager {
public:
    ShmManager();
    ~ShmManager();

    // 初始化SHM资源对象，即创建/打开
    bool Init(uint64_t shm_size, bool is_manager);
    // 释放SHM资源，即销毁/关闭
    void Release();

    void* GetMemory(uint64_t& mem_size);

private:
    bool is_manager_{false};
    int shm_fd_{-1};
    void* start_ptr_{nullptr};
    uint64_t shm_size_{0};

    static const std::string shm_name_;
};
