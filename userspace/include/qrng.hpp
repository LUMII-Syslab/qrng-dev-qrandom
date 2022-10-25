#pragma once

#include "rng.hpp"
#include <qrng.h>

class QRNG : public RNG{
private:
    graal_isolatethread_t* thread;
public:
    QRNG();
    ~QRNG();
    void fetch_bytes(char b_arr[], int b_cnt);
};