#pragma once

#include <stdint.h>
#include <string>

#include "circular_queue.h"
#include "shm_manager.h"

class ShmQueue {
public:
    ShmQueue();
    ~ShmQueue();

    bool Init(uint64_t shm_size, bool is_manager);
    bool Push(const std::string& data);
    bool Pop(std::string& data);

private:
    ShmManager shm_manager_;
    CircularQueue circular_queue_;
};
