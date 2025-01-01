#include "shm_queue.h"

#include <iostream>


ShmQueue::ShmQueue() {
}

ShmQueue::~ShmQueue() {
    circular_queue_.Release();
    shm_manager_.Release();
}

bool ShmQueue::Init(uint64_t shm_size, bool is_manager) {
    if (!shm_manager_.Init(shm_size, is_manager)) {
        return false;
    }

    uint64_t mem_size = 0;
    void* shm_ptr = shm_manager_.GetMemory(mem_size);
    if (shm_ptr == nullptr) {
        return false;
    }

    if (!circular_queue_.Init(shm_ptr, mem_size, is_manager)) {
        return false;
    }
    return true;
}

bool ShmQueue::Push(const std::string& data) {
    return circular_queue_.Push(data);
}

bool ShmQueue::Pop(std::string& data) {
    return circular_queue_.Pop(data);
}
