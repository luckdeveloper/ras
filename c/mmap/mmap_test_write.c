#include <stdio.h>  
#include <unistd.h>  
#include <sys/mman.h>  
#include <sys/types.h>  
#include <fcntl.h>  
#include <stdlib.h>  
#include <errno.h>

#define PAGE_SIZE (4096)
#define LEN (10*PAGE_SIZE)


int main(void)
{
    int fd;
    int i;
    char *virtual_addr;

    if ((fd = open("/dev/mmaptest", O_RDWR)) < 0)
    {
        printf("open device /dev/mmaptest failed, errno %d\n", errno);
        return 0;
    }
    virtual_addr = mmap(0, LEN, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, fd, 0);

    sprintf(virtual_addr, "write from userspace at virtual address: %lx", virtual_addr); 

#if 0
    for (i = 0; i < 10; i++)
    {
        sprintf(virtual_addr, "write from userspace at virtual address: %lx", virtual_addr); 
        virtual_addr += PAGE_SIZE * i;       
    }
#endif

    while (1)
    {
        sleep(1);
    }
    return 0;
}