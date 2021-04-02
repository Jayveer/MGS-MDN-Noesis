#pragma once

inline
int32_t negateBits(int32_t val, int numBits) {
    int hi = numBits - 1;
    int32_t mx = pow(2, hi);
    return val >> hi ? val |= -mx : val;
}