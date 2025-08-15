#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEM_SIZE (256 * 1024 * 1024) 

int main() {
    void *allocated_mem;

    allocated_mem = mmap(NULL, MEM_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (allocated_mem == MAP_FAILED) {
        perror("Memory mapping failed");
        return EXIT_FAILURE;
    }

    for (size_t offset = 0; offset < MEM_SIZE; offset += 4096) {
        ((char *)allocated_mem)[offset] = 0; 
    }

    printf("Reserved %d MiB of anonymous memory at address %p\n", MEM_SIZE / (1024 * 1024), allocated_mem);
    printf("Current process PID: %d\n", getpid());

    getchar();

    return 0;
}
