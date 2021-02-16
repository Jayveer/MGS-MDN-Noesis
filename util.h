#pragma once

inline
float byteswap_float(float in) {
    float out = 0;
    char* inFloat = (char*)&in;
    char* outFloat = (char*)&out;

    outFloat[0] = inFloat[3];
    outFloat[1] = inFloat[2];
    outFloat[2] = inFloat[1];
    outFloat[3] = inFloat[0];

    return out;
}