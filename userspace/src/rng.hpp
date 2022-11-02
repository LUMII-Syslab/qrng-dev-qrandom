#pragma once

class RNG {
    public:
        virtual void fetch_bytes(char b_arr[], int b_cnt) = 0;
};