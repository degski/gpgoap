#ifndef BINARY_ADD_INCLUDED
#define BINARY_ADD_INCLUDED

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define B_TRUE 1
#define B_FALSE 0

typedef uint8_t bit;

inline bit B_AND(bit a, bit b) {
    bit res = (a == B_TRUE && b == B_TRUE);

    return res & B_TRUE;
}

inline bit B_OR(bit a, bit b) {
    bit res = (a == B_TRUE || b == B_TRUE);

    return res & B_TRUE;
}

inline bit B_XOR(bit a, bit b) {
    bit res = ((a == B_TRUE && b == B_FALSE) || (a == B_FALSE && b == B_TRUE));

    return res & B_TRUE;
}

inline bit B_NOT(bit a) {
    return (a == B_FALSE) & B_TRUE;
}

bit B_FA(bit a, bit b, bit cin) {
    if (B_XOR(B_XOR(a, b), cin) == B_TRUE) {
        return B_TRUE;
    } else {
        return B_FALSE;
    }
}

bit B_OVF(bit a, bit b, bit cin) {
    if (B_OR(B_AND(a, b), B_AND(B_XOR(a, b), cin)) == B_TRUE) {
        return B_TRUE;
    } else {
        return B_FALSE; 
    }
}

bit bitAt(int16_t a, int16_t idx) {
    return (bit)((a & (1 << idx)) >> idx);
}

int16_t binaryAdd(int16_t a, int16_t b) {
    uint8_t i;

    int16_t res = 0;
    bit cin = B_FALSE;
    for (i = 0; i < 16; ++ i) {
        bit ba = bitAt(a, i);
        bit bb = bitAt(b, i);

        res |= ((uint16_t) B_FA(ba, bb, cin)) << i;
        cin = B_OVF(ba, bb, cin);
    }

    return res;
}

char* showBinary(int16_t a) {
    int i;
    
    char *bit_str = (char*) malloc(sizeof(char) * 32);
    for (i = 15; i >= 0; -- i) {
        bit_str[15 - i] = bitAt(a, i) + 48;
    }

    return bit_str;
}

#endif
