#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "loguru/loguru.hpp"

#include "rng.hpp"
#include "prng.hpp"
#include "qrng.hpp"

#define DEVICE_PATH "/dev/qrandom0"
#define BLOCK_SZ  256 // should match the value defined in kernelspace

RNG* rng = new PRNG();  // random num gen used to write /dev/qrandom0
                        // PRNG can be replaced by QRNG

void write_random_bytes(int fd, RNG* rng) {

    LOG_F(INFO,"preparing %d random bytes", BLOCK_SZ);

    byte bytes[BLOCK_SZ];
    rng->fetch_bytes(bytes,BLOCK_SZ);

    LOG_F(INFO,"writing %d bytes to %s", BLOCK_SZ, DEVICE_PATH);

    int wres = write(fd,bytes,sizeof(bytes));

    if(wres<0)
        ABORT_F("write failed with errno %d", errno);
    else
        LOG_F(INFO, "%d bytes written to %s\n",wres,DEVICE_PATH);
}

int main(int argc, char **argv) {
    loguru::init(argc,argv);
    loguru::add_file("qrng.log", loguru::FileMode::Truncate, loguru::NamedVerbosity::Verbosity_INFO);

    int fd = open(DEVICE_PATH, O_RDWR);

    if(fd<0) {
        if(errno == 13) ABORT_F("open failed with permission denied");
        else ABORT_F("open failed with errno %d", errno);
        return errno;
    }

    LOG_F(INFO,"succesfully opened %s for writing", DEVICE_PATH);

    pollfd qrngpoll = {};
    qrngpoll.fd = fd;
    qrngpoll.events = POLLOUT;

    while(1) {
        LOG_F(INFO,"polling for POLLOUT event");
        poll(&qrngpoll,1,10000); // poll for 10 seconds and then poll again
        if(qrngpoll.revents & POLLOUT) {
            qrngpoll.revents = 0;

            LOG_F(INFO, "random bytes should be written");

            write_random_bytes(fd,rng);
        } else {
            LOG_F(INFO, "poll timed out - buffer probably full");
        }
    }
    return 0;
}