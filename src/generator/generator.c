#include <stdlib.h>
#include <time.h>

void initRandomSeed() {
    srand(time(NULL));
}

float zeroGenerator() {
    return 0.0;
}

float biasGenerator() {
    return ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
}

float matrixGenerator() {
    return ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
}

float tensorGenerator() {
    return ((float)rand() / RAND_MAX - 0.5f) * 0.1f;
}