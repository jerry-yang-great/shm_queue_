#pragma once

#include <stdint.h>
#include <string>

#include "header.h"
#include "shm_mutex.h"

class CircularQueue {
public:
    CircularQueue();
    ~CircularQueue();

    bool Init(void* ptr, uint64_t size, bool is_manager);
    void Release();

    bool Push(const std::string& data);
    bool Pop(std::string& data);

private:
    QueueHeader* header_{nullptr};
    uint8_t* buffer_ptr_{nullptr};
    uint64_t buffer_size_{0};
    SHMMutex shm_mutex_;

    uint64_t GetOffset(uint64_t offset);
};
