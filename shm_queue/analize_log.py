#!--coding : utf-8--
'''
这个脚本用来分析多个读进程读取的数据，确认读取到的所有数据和写入的数据一致，数据没有重复，没有缺失。
具体操作步骤入下：
1. 编译：
./compile.sh
2. 运行程序
./test_shm_queue a & sleep 0.1 &&./test_shm_queue > 1.log& sleep 0.1 &&./test_shm_queue >2.log &
3. 运行此脚本
python3 analize_log.py
4. 观察结果
预期没有重复数据、没有缺失数据。

'''

def read_integers_from_file(file_path):  
    integers = set()  
    try:  
        with open(file_path, 'r') as file:  
            for line in file:  
                try:  
                    integer = int(line.strip())  
                    integers.add(integer)  
                except ValueError:  
                    # Skip lines that are not integers  
                    continue  
    except FileNotFoundError:  
        print(f"Error: File {file_path} not found.")  
    return integers  
  
def find_common_and_missing_integers(file1, file2, n):  
    integers_file1 = read_integers_from_file(file1)  
    integers_file2 = read_integers_from_file(file2)  
      
    common_integers = integers_file1.intersection(integers_file2)  
    all_integers = integers_file1.union(integers_file2)  
      
    missing_integers = set(range(0, n + 1)) - all_integers  
      
    return common_integers, missing_integers  
  
# 使用示例  
file1 = '1.log'  
file2 = '2.log'  
n = 3000000-1  # 假设范围是0到2999999，可以根据需要调整.
  
common_integers, missing_integers = find_common_and_missing_integers(file1, file2, n)

sorted_list = sorted(common_integers)
print("sorted_list:", sorted_list)
print("在两个文件中都有的整数:", common_integers)
print("在两个文件中都没有的整数 (0~{}):".format(n), missing_integers)
