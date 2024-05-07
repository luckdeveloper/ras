#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>  
#include <sys/mman.h>  
#include <sys/stat.h>  
#include <string.h>  
#include <unistd.h>  
  
#define FILE_PATH "test.txt"  
#define FILE_SIZE 1024  // 假设文件大小为1024字节，实际应根据文件大小调整  
#define SHARED_MEM_SIZE 1024  // 共享内存的大小  

void *gen_file_mmap_exam();
void *gen_anony_mmap_exam();

int main() {  
 
    void *mapped_file = NULL;
    void *mapped_shared_memory = NULL;

    mapped_file = gen_file_mmap_exam();
    mapped_shared_memory = gen_anony_mmap_exam();
    printf("mapped_file: %p\n", mapped_file);
    printf("mapped_shared_memory: %p\n", mapped_shared_memory);
    
    while (1)
    {
        sleep(1);
    }

    return 0;  
}

void *gen_file_mmap_exam()
{
    int fd;  
    struct stat sb;  
    void *mapped_file = MAP_FAILED;  
  
    // 打开文件  
    fd = open(FILE_PATH, O_RDWR | O_CREAT, (mode_t)0600);  
    if (fd == -1) {  
        perror("open");  
        exit(EXIT_FAILURE);  
    }  
  
    // 写入一些数据到文件（可选）  
    // 假设我们只是简单地写入一个字符串  
    const char *str = "Hello, mmap!";  
    write(fd, str, strlen(str));  
  
    // 获取文件大小  
    if (fstat(fd, &sb) == -1) {  
        perror("fstat");  
        exit(EXIT_FAILURE);  
    }  
  
    // 确保文件大小不超过我们定义的FILE_SIZE  
    if (sb.st_size > FILE_SIZE) {  
        fprintf(stderr, "File is too large.\n");  
        exit(EXIT_FAILURE);  
    }  
  
    // 使用mmap映射文件到内存  
    mapped_file = mmap(0, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);  
    if (mapped_file == MAP_FAILED) {  
        perror("mmap");  
        exit(EXIT_FAILURE);  
    }  
  
    // 现在可以在内存中直接访问文件数据了  
    printf("File content in memory: %s\n", (char *)mapped_file);  // 假设文件内容以字符串形式存储  
  
    // 在内存中修改文件内容（可选）  
    strcpy((char *)mapped_file, "Updated content!");  
  
    // 通知系统文件内容已被修改（可选，取决于是否需要立即将更改写回磁盘）  
    // msync(mapped_file, sb.st_size, MS_SYNC);  

    return mapped_file;

#if 0
    // 解除映射  
    if (munmap(mapped_file, sb.st_size) == -1) {  
        perror("munmap");  
        exit(EXIT_FAILURE);  
    }  
  
    // 关闭文件  
    close(fd);  
  
    // 这里的代码只是示例，通常你可能希望检查`msync`的返回值以确保数据已成功写回磁盘  
  
    return 0;
#endif

}

void *gen_anony_mmap_exam()
{
    void *shared_memory = mmap(NULL, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);  
    if (shared_memory == MAP_FAILED) {  
        perror("mmap");  
        exit(EXIT_FAILURE);  
    }  
    
    const char *str = "Hello, mmap!";  
    memcpy(shared_memory, str, strlen(str));

    return shared_memory;
}