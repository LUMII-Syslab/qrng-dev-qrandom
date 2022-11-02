#pragma once

typedef unsigned char byte;

/// @brief random number generator interface
class RNG {
    public:
        virtual void fetch_bytes(byte arr[], int cnt) = 0;
};