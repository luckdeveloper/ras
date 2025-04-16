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
    int fd, loop;
    char *vadr;

    if ((fd = open("/dev/mmaptest", O_RDWR)) < 0)
    {
        printf("open device /dev/mmaptest failed, errno is %d\n", errno);
        return 0;
    }
    vadr = mmap(0, LEN, PROT_READ, MAP_PRIVATE | MAP_LOCKED, fd, 0);
    for (loop = 0; loop < 10; loop++)
    {
        printf("[%-10s---- access address: %lx]\n", 
            vadr + PAGE_SIZE * loop, 
            vadr + PAGE_SIZE * loop);
    }
    while (1)
    {
        sleep(1);
    }
}