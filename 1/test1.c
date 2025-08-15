#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_pages> <stride_in_MB>\n", argv[0]);
        return 1;
    }

    int pages = atoi(argv[1]);
    int stride_mb = atoi(argv[2]);
    long pagesize = sysconf(_SC_PAGE_SIZE);
    
    int stride = stride_mb * 1024 * 1024;

    void *alloc_memory = mmap(NULL, pages * pagesize, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (alloc_memory == MAP_FAILED) {
        perror("mmap failed");
        return 1;
    }

    unsigned char *ptr = (unsigned char *)alloc_memory;
    for (int i = 0; i < pages * pagesize; i += stride) {
        ptr[i] = 0xAA;  
    }
    printf("PID: %u\n", getpid());
    printf("Stride: %d MB , Pages: %d pages\n", stride_mb,  pages);

    getchar();

    munmap(alloc_memory, pages * pagesize);
    return 0;
}

