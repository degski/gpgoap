#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "binary.h"
#include "ternary.h"

#define START -700
#define RANGE 1400


int main() {
    int16_t a = 1, b = -1, i, j;

    int16_t  binary_samples[RANGE], ternary_samples[RANGE],
             binary_results[RANGE * RANGE], ternary_results[RANGE * RANGE];

    printf("Initialization\n");
    clock_t start_init = clock();
    for (i = 0, j = START; i < RANGE; ++ i, ++ j) {
        binary_samples[i] = j;
        ternary_samples[i] = decToTer(j);
    }

    // Addition Test (binary)
    printf("Binary Add\n");
    clock_t start_binary_add = clock();
    for (i = 0; i < RANGE; ++ i) {
        for (j = 0; j < RANGE; ++ j) {
            binary_results[i * RANGE + j] = binaryAdd(binary_samples[i], binary_samples[j]);
        }
    }

    // Addition Test (ternary)
    printf("Ternary Add\n");
    clock_t start_ternary_add = clock();
    for (i = 0; i < RANGE; ++ i) {
        for (j = 0; j < RANGE; ++ j) {
            ternary_results[i * RANGE + j] = ternaryAdd(ternary_samples[i], ternary_samples[j]);
        }
    }

    clock_t end = clock();

    // Check
    int16_t res;
    int ind;
    for (i = 0; i < RANGE; ++ i) {
        for (j = 0; j < RANGE; ++ j) {
            res = binary_samples[i] + binary_samples[j];
            ind = i * RANGE + j;

            if (binary_results[ind] != res) {
                printf("binary error!!\n");
            }

            if (terToDec(ternary_results[ind]) != res) {
                printf("ternary error!!\n");
            }
        }
    }

    // Profile timing
    printf("Initialization: %lf\n", (double)(start_binary_add - start_init) / CLOCKS_PER_SEC);
    printf("Binary Add: %lf\n", (double)(start_ternary_add - start_binary_add) / CLOCKS_PER_SEC);
    printf("Ternary Add: %lf\n", (double)(end - start_ternary_add) / CLOCKS_PER_SEC);


    return 0;
}
