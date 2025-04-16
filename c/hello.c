#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "inttypes.h"

uintptr_t vtop(uintptr_t vaddr) {
    FILE *pagemap;
    intptr_t paddr = 0;
    int offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e;

    // https://www.kernel.org/doc/Documentation/vm/pagemap.txt
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {
                if (e & (1ULL << 63)) { // page present ?
                    paddr = e & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    // add offset within page
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }   
            }   
        }   
        fclose(pagemap);
    }   

    return paddr;
}   


int main(){
	char * str = (char*)malloc(10000*sizeof(char));
	char * str1 = (char*)malloc(10000*sizeof(char));
	int count = 0;
	strcpy(str, "Hello world!\n");
	while(1){
		count += 1;
		printf("count -> %d\n",count);
		printf("%s", str);
		printf("used va -> %p\n",str);
		printf("unused va -> %p\n",str1);
        #if 0
		if (count == 50){
			strcpy(str1, "Hello world!\n");
		}
        #endif 
		sleep(2);		
	}
	free(str);
	return 0;
}





