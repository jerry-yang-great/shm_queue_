# shm_queue_
A queue based on the shared memory. a IPC queue.
具体操作步骤入下：
1. 编译：
./compile.sh
2. 运行程序
./test_shm_queue a & sleep 0.1 &&./test_shm_queue > 1.log& sleep 0.1 &&./test_shm_queue >2.log &
3. 运行此脚本
python3 analize_log.py
4. 观察结果
预期没有重复数据、没有缺失数据。