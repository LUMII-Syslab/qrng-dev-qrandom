#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>

#include "loguru/loguru.hpp"

#include "rng.hpp"
#include "prng.hpp"
#include "qrng.hpp"

#define DEVICE_PATH "/dev/qrandom0"

#define REQUEST_SIZE 516 // requested bytes from qrng service
#define WRITE_SIZE 256   // passed bytes to qrng device

RNG *rng = new PRNG(); // random num gen used to write /dev/qrandom0
                       // PRNG can be replaced by QRNG

byte qrng_buff[REQUEST_SIZE]; // saves fetched bytes from request to qrng.lumii.lv
int qrng_buff_read_it = REQUEST_SIZE; // when read_it is REQUEST_SIZE, buff is reset

byte qrng_write_arr[WRITE_SIZE]; // array for writing to qrng device
int qrng_write_arr_renew = WRITE_SIZE; // how many bytes should be renewed in the array

void log_bytes(byte bytes[], int byte_cnt) {
    std::string bytes_str = "";
    for (int i = 0; i < byte_cnt; i++)
        bytes_str += std::to_string(bytes[i]), bytes_str += " ";
    LOG_F(INFO, "bytes: %s", bytes_str.c_str());
}

void write_random_bytes(int fd, RNG *rng)
{ 
    LOG_F(INFO, "preparing %d random bytes", WRITE_SIZE);

    int qrng_write_arr_it = 0;
    while(qrng_write_arr_it<qrng_write_arr_renew) {

        if(qrng_buff_read_it==REQUEST_SIZE) {
            LOG_F(INFO, "requesting %d random bytes", REQUEST_SIZE);
            rng->fetch_bytes(qrng_buff, REQUEST_SIZE);
            qrng_buff_read_it = 0;
        }

        qrng_write_arr[qrng_write_arr_it++] = qrng_buff[qrng_buff_read_it++];
    }

    LOG_F(INFO, "writing %d bytes to %s", WRITE_SIZE, DEVICE_PATH);
    int wres = write(fd, qrng_write_arr, sizeof(qrng_write_arr));

    if (wres < 0)
        ABORT_F("write failed with errno %d", errno);
    
    qrng_write_arr_renew = wres;
    LOG_F(INFO, "%d bytes written to %s\n", wres, DEVICE_PATH);
}

int main(int argc, char **argv)
{
    loguru::init(argc, argv);
    // loguru::add_file("qrng.log", loguru::FileMode::Truncate, loguru::NamedVerbosity::Verbosity_INFO);

    int fd = open(DEVICE_PATH, O_RDWR);

    if (fd < 0)
    {
        if (errno == 13)
            ABORT_F("open failed with permission denied");
        else
            ABORT_F("open failed with errno %d", errno);
        return errno;
    }

    LOG_F(INFO, "succesfully opened %s for writing", DEVICE_PATH);

    pollfd qrngpoll = {};
    qrngpoll.fd = fd;
    qrngpoll.events = POLLOUT;

    while (1)
    {
        LOG_F(INFO, "polling for POLLOUT event");
        int r = poll(&qrngpoll, 1, -1); // poll single file descriptor wihtout timeout
        if (qrngpoll.revents & POLLOUT)
        {
            qrngpoll.revents = 0;

            LOG_F(INFO, "random bytes should be written");

            write_random_bytes(fd, rng);
        }
    }
    return 0;
}