#!/bin/bash

g++ shm_queue.cpp shm_manager.cpp circular_queue.cpp shm_mutex.cpp main.cpp -o test_shm_queue -lpthread -g -O0