#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "rng.hpp"
#include "prng.hpp"
#include "qrng.hpp"

#define DEVICE_PATH "/dev/qrandom0"
#define BLOCK_SZ  256 // must match the value defined in kernelspace

RNG* rng; // random num gen used to write /dev/qrandom0

void write_random_bytes(int fd, RNG* rng) {
    byte bytes[BLOCK_SZ];
    
    rng->fetch_bytes(bytes,BLOCK_SZ);
    int wres = write(fd,bytes,sizeof(bytes));

    if(wres<0) {
        printf("write failed with errno %d\n", errno);
        exit(1);
    }else
        printf("%d bytes written to %s\n",wres,DEVICE_PATH);
}

int main(int argc, char **argv) {
    rng = new PRNG(); // can be swapped out for QRNG

    int fd = open(DEVICE_PATH, O_RDWR);
    if(fd<0) {
        if(errno == 13) printf("%s failed with permission denied");
        else printf("%s failed with errno %d\n", DEVICE_PATH, errno);
        return errno;
    }

    pollfd qrngpoll = {};
    qrngpoll.fd = fd;
    qrngpoll.events = POLLOUT;

    while(1) {
        poll(&qrngpoll,1,1000);
        if(qrngpoll.revents & POLLOUT) {
            qrngpoll.revents = 0;
            printf("%s is ready for writing\n", DEVICE_PATH);
        } else {
            printf("poll timed out\n");
        }
    }
    return 0;
}