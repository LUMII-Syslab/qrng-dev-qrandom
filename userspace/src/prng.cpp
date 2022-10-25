#include "prng.hpp"
#include <stdio.h>
#include <chrono>
#include <string.h>


PRNG::PRNG() {
    this->mt = new std::mt19937(
        std::chrono::steady_clock::now()
        .time_since_epoch().count());
}

PRNG::~PRNG() {
    delete this->mt;
}

void PRNG::fetch_bytes(char b_arr[], int b_cnt) {
    while(b_cnt>0){
        int rnd = this->mt->operator()();
        char tmp[sizeof(rnd)];
        memcpy(tmp,&rnd,sizeof(rnd));
        int i=sizeof(rnd);
        while(b_cnt&&i)
            b_arr[--b_cnt] = tmp[--i];
    }
}