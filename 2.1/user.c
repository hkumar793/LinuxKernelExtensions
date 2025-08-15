#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
 
#define get_PA_from_VA  _IOR('p', 1, unsigned long)
#define wrt_data_to_PA _IOW('p', 2, unsigned long)

typedef struct {
    unsigned long pa;
    unsigned char data;
} pa_struct;

int main(int argc, char *argv[]) {

    if (argc != 2) { 
        fprintf(stderr, "Usage: %s <count>\n", argv[0]);
        return 1;
    }

    int fd = open("/dev/ioctl_device_driver", O_RDWR);
    if (fd < 0) {
        perror("Error opening device");
        return 1;
    }

    char *input_arg = argv[1]; 
    char *endptr;
    size_t count = strtoul(input_arg, &endptr, 10);

    unsigned char *bck = malloc(count);
    if (!bck) {
        perror("Memory allocation failed");
        close(fd);
        return 1;
    }

    for (size_t i = 0; i < count; i++) {
        bck[i] = 104 + i;
    }

    printf("\n");
    for (size_t i = 0; i < count; i++) {
        printf("VA: %p      Value: %d\n", &bck[i], bck[i]);
    }

    printf("\nVA to PA Translation:\n");
    unsigned long phy_adress[count];
    for (size_t i = 0; i < count; i++) {
        unsigned long va = (unsigned long)&bck[i];
        if (ioctl(fd, get_PA_from_VA, &va) < 0) {
            perror("get_PA_from_VA failed");
            free(bck);
            close(fd);
            return 1;
        }
        phy_adress[i] = va;
        printf("VA: %p      PA: 0x%lx\n", &bck[i], va);
    }
    printf("\n");

    for (size_t i = 0; i < count; i++) {
        printf("PA: 0x%lx       Value: %d\n", phy_adress[i], bck[i]);
    }
    printf("\n");

    for (size_t i = 0; i < count; i++) {
        pa_struct pa_struct_var = {phy_adress[i], 53 + (unsigned char)i};
        if (ioctl(fd, wrt_data_to_PA, &pa_struct_var) < 0) {
            perror("wrt_data_to_PA failed");
            free(bck);
            close(fd);
            return 1;
        }
        printf("VA: %p      PA: 0x%lx      Value: %d\n", &bck[i], phy_adress[i], pa_struct_var.data);
    }

    free(bck);
    close(fd);
    return 0;
}


