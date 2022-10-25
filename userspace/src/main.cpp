#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include "prng.hpp"
#include "qrng.hpp"

RNG* rng; // random num gen used to write /dev/qrandom0

int main(int argc, char **argv) {
    rng = new PRNG(); // can be swapped out for QRNG

    char b_arr[1000];
    int b_cnt = sizeof(b_arr)/sizeof(b_arr[0]);
    rng->fetch_bytes(b_arr,b_cnt);

    for(int i=0;i<b_cnt;i++)
        printf("%d ", static_cast<unsigned char>(b_arr[i]));

    return 0;
}