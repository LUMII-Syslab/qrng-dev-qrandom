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

void PRNG::fetch_bytes(byte arr[], int cnt) {
    while(cnt>0){
        int rnd = this->mt->operator()();
        
        byte tmp[sizeof(rnd)]; // because int contains multiple bytes
        memcpy(tmp,&rnd,sizeof(rnd));

        int i=sizeof(rnd);
        while(cnt&&i)
            arr[--cnt] = tmp[--i];
    }
}