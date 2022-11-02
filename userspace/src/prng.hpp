#pragma once

#include "rng.hpp"
#include <random>

class PRNG : public RNG {
private:
    std::mt19937* mt;
public:
    PRNG();
    ~PRNG();
    void fetch_bytes(char b_arr[], int b_cnt);
};