#include "circular_queue.h"

//#include <fcntl.h>  // For O_* constants
//#include <sys/mman.h>
//#include <unistd.h>

#include <iostream>
#include <mutex>
#include <cstring>

CircularQueue::CircularQueue() {
}

CircularQueue::~CircularQueue() {
    Release();
}

bool CircularQueue::Init(void* ptr, uint64_t size, bool is_manager) {
    if (is_manager) {
        header_ = new (ptr) QueueHeader();
    } else {
        header_ = (QueueHeader*) ptr;
    }
    shm_mutex_.Init(&header_->mutex);
    buffer_ptr_ = (uint8_t*) (header_+1);
    buffer_size_ = size - sizeof(QueueHeader);
    return true;
}

void CircularQueue::Release() {
    header_ = nullptr;
    buffer_ptr_ = nullptr;
    buffer_size_ = 0;
    shm_mutex_.Release();
}

bool CircularQueue::Push(const std::string& data) {
    // 1. 数据校验
    if (data.size() == 0) {
        return false;
    }

    if (!header_ || !buffer_ptr_) {
        std::cerr << "header_ is nullptr" << std::endl;
        return false;
    }

    if (buffer_size_ < data.size() + sizeof(Message)) {
        std::cerr << "buffer_size_ < data.size() + \
            sizeof(Message)" << std::endl;
        return false;
    }

    // 2. 同步加锁
    std::lock_guard<SHMMutex> lg(shm_mutex_);
    // 3. 队列溢出检查
    uint64_t head = header_->head.load();
    uint64_t old_tail = header_->tail.load();
    uint64_t new_tail = old_tail + data.size() + sizeof(Message);
    uint64_t new_tail_offset = GetOffset(new_tail);
    if ((old_tail < head && head <= new_tail) ||
        (old_tail > head && head <= new_tail_offset &&
        new_tail_offset < new_tail)) {
        //这里的<=，使得队列满时最大容量是buffer_size_-1
        //std::cerr << "shm_queue is full." << std::endl;
        return false; // 队列满了
    }

    // 4. 数据拷贝
    if (new_tail <= buffer_size_) {
        Message* msg = (Message*) (buffer_ptr_ + old_tail);
        memcpy(msg->data, data.data(), data.size());
        msg->length = data.size();
    } else { // 跨越了buffer的结尾
        Message* msg = (Message*) (buffer_ptr_ + old_tail);
        uint64_t first_len = buffer_size_ - old_tail;
        if (first_len >= sizeof(Message)) {
            // data跨越了buffer尾部
            first_len = first_len - sizeof(Message);
            memcpy(msg->data, data.data(), first_len);
            uint64_t second_len = data.size() - first_len;
            memcpy(buffer_ptr_, data.data() + first_len,
                second_len);
            msg->length = data.size();
        } else { // Message跨越了buffer尾部
            Message msg_tmp;
            msg_tmp.length = data.size();
            memcpy((char*)msg, (char*)&msg_tmp, first_len);
            uint64_t second_len = sizeof(Message) - first_len;
            memcpy(buffer_ptr_, ((char*)&msg_tmp) + first_len,
                second_len);
            memcpy(buffer_ptr_+second_len, data.data(),
                data.size());
        }
    }

    // 5. 修改队尾指针
    header_->tail.store(new_tail_offset);
    // 6. 解锁，lg析构自动解锁
    return true;
}

bool CircularQueue::Pop(std::string& data) {
    data.clear();
    if (!header_ || !buffer_ptr_) {
        std::cerr << "header_ is nullptr" << std::endl;
        return false;
    }

    // 1. 同步加锁
    std::lock_guard<SHMMutex> lg(shm_mutex_);
    // 2. 检查队列是否为空
    uint64_t head = header_->head.load();
    if (head == header_->tail.load()) {
        //std::cerr << "shm_queue is empty." << std::endl;
        return false; // 队列空了
    }

    // 3.数据读出拷贝
    Message* msg = (Message*) (buffer_ptr_ + head);
    uint64_t new_head = 0;
    if (buffer_size_ - head >= sizeof(Message)) {
        // 在Message头部是完整的情况下
        uint64_t msg_len = sizeof(Message) + msg->length;
        if (msg->length >= buffer_size_ ||
                msg_len >= buffer_size_) {
            std::cerr << "invalid data." << std::endl;
            return false;
        }

        new_head = GetOffset(head + msg_len);
        if (head + msg_len <= buffer_size_) {
            data.assign((char*)msg->data, msg->length);
        } else {
            uint64_t first_len = buffer_size_ - head
                - sizeof(Message);
            uint64_t second_len = msg->length - first_len;
            data.reserve(msg->length);
            data.assign((char*)msg->data, first_len);
            data.append((char*)buffer_ptr_, second_len);
        }
    } else { // Message头部被拆分时
        uint64_t first_len = buffer_size_ - head;
        Message msg_tmp;
        memcpy(&msg_tmp, msg, first_len);
        uint64_t second_len = sizeof(Message) - first_len;
        memcpy(((char*)&msg_tmp)+first_len, buffer_ptr_,
            second_len);
        
        if (msg_tmp.length >= buffer_size_ ||
            msg_tmp.length + sizeof(Message) >= buffer_size_) {
            std::cerr << "invalid data." << std::endl;
            return false;
        }
        
        data.append((char*)buffer_ptr_+second_len,
            msg_tmp.length);
        new_head = second_len + msg_tmp.length;
    }

    // 4. 修改队头指针
    header_->head.store(new_head);
    // 5. 解锁，lg析构自动解锁
    return true;
}

uint64_t CircularQueue::GetOffset(uint64_t offset) {
    return offset % buffer_size_;
}

