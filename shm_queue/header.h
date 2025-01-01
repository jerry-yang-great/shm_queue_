#pragma once

#include <atomic>
#include <pthread.h>
#include <stdint.h>

struct QueueHeader {
    pthread_mutex_t mutex;
    std::atomic_int64_t head{0};
    std::atomic_int64_t tail{0};
};

struct Message{
    uint32_t length{0};
    uint8_t data[0];
};