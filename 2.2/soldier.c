#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

// Include header or define the IOCTL call interface and devide name

#define DRIVER_NAME "/dev/chardev"
#define PARENT_CHG _IOW('a', 1, pid_t)

//**************************************************

int open_driver(const char* driver_name) {

    int fd_driver = open(driver_name, O_RDWR);
    if (fd_driver == -1) {
        perror("ERROR: could not open driver");
    }

	return fd_driver;
}

void close_driver(const char* driver_name, int fd_driver) {

    int result = close(fd_driver);
    if (result == -1) {
        perror("ERROR: could not close driver");
    }
}


int main(int argc, char** argv) {

    if (argc != 2) {
        printf("Usage: %s <parent_pid>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t parent_pid = atoi(argv[1]);


    // open ioctl driver
    int driver_fd = open_driver(DRIVER_NAME);
    if (driver_fd == -1) {
        exit(EXIT_FAILURE);
    }  
    
    // call ioctl with parent pid as argument to change the parent
    printf("[CHILD]: soldier %d changing its parent\n", getpid());
    if (ioctl(driver_fd, PARENT_CHG, &parent_pid) == -1) {
        perror("ERROR: ioctl failed for IOCTL_CHANGE_PARENT");
        close_driver(DRIVER_NAME, driver_fd);
        exit(EXIT_FAILURE);
    }
	
    // close ioctl driver
    close_driver(DRIVER_NAME, driver_fd);

    while(1) {
        sleep(1);
    }

	return EXIT_SUCCESS;
}


