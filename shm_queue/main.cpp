#include <iostream>
#include <set>

#include "shm_queue.h"

uint64_t const SHM_SIZE=1024*1024*10;

int main_simple(int argc, char* argv[]) {
    bool is_manger = argc >1;
    ShmQueue shm_queue;
    shm_queue.Init(SHM_SIZE, is_manger);

    std::string str("long int atol（const char * str）");
    if (!shm_queue.Push(str)) {
        std::cout << "shm_queue.WriteBuffer failed" << std::endl;
    }
    std::string str1("int main(int argc, char* argv[]) {");
    if (!shm_queue.Push(str1)) {
        std::cout << "shm_queue.WriteBuffer failed" << std::endl;
    }

    std::string str_out;
    if (!shm_queue.Pop(str_out)) {
        std::cout << "shm_queue.ReadBuffer failed" << std::endl;
    }
    std::string str_out1;
    if (!shm_queue.Pop(str_out1)) {
        std::cout << "shm_queue.ReadBuffer failed" << std::endl;
    }

    if(str.compare(str_out)==0) {
        std::cout << "shm_queue.ReadBuffer equal to shm_queue.WriteBuffer" << std::endl;
    } else {
        std::cout << "shm_queue is not consistent" << std::endl;
    }
    if(str1.compare(str_out1)==0) {
        std::cout << "shm_queue.ReadBuffer equal to shm_queue.WriteBuffer" << std::endl;
    } else {
        std::cout << "shm_queue is not consistent" << std::endl;
    }

    if (shm_queue.Pop(str_out1)) {
        std::cout << "unexpected action for shm_queue.ReadBuffer" << std::endl;
    }
    return 0;
}

const int CountLimit = 3000000;

void WriteProcess(ShmQueue& shm_queue) {
    for (int i = 0; i < CountLimit; ++i) {
        std::string str = std::to_string(i);
        while(!shm_queue.Push(str)) {
            usleep(10);
        }
    }
}

void ReadProcess(ShmQueue& shm_queue) {
    for (int i = 0; i < CountLimit; ++i) {
        std::string str = std::to_string(i);
        std::string str_out;
        while(!shm_queue.Pop(str_out)) {
            usleep(10);
        }
        if (str.compare(str_out) != 0) {
            std::cout << "shm_queue is not consistent, str=" << str
                << ", str_out=" << str_out << std::endl;
        }
    }
}

void ReadPrintProcess(ShmQueue& shm_queue) {
    const int retry_count_limit = 100*100;
    std::set<std::string> read_set;
    for (int i = 0; i < CountLimit; ++i) {
        std::string str = std::to_string(i);
        std::string str_out;
        int retry_count = 0;
        while(!shm_queue.Pop(str_out) && retry_count <= retry_count_limit) {
            usleep(10);
            ++ retry_count;
        }
        if (retry_count > retry_count_limit && str_out.empty()) {
            break;
        }
        read_set.emplace(str_out);
    }

    for (auto& out: read_set) {
        std::cout << out << std::endl;
    }
}

int main_two_process(int argc, char* argv[]) {
    bool is_manger = argc >1;
    ShmQueue shm_queue;
    shm_queue.Init(SHM_SIZE, is_manger);

    if(is_manger) {
        WriteProcess(shm_queue);
    } else {
        ReadProcess(shm_queue);
    }

    return 0;
}

// 
int main_n_process(int argc, char* argv[]) {
    bool is_manger = argc >1;
    ShmQueue shm_queue;
    shm_queue.Init(SHM_SIZE, is_manger);

    if(is_manger) {
        WriteProcess(shm_queue);
    } else {
        ReadPrintProcess(shm_queue);
    }

    return 0;
}

int main(int argc, char* argv[]) {
    // return main_simple(argc,argv);
    // return main_two_process(argc,argv);
    return main_n_process(argc,argv);
}