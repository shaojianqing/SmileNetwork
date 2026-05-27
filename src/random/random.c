#include <stdlib.h>
#include <time.h>

void initRandomSeed() {
    srand(time(NULL));
}

float biasGenerator() {
    return rand()/RAND_MAX;
}

float matrixGenerator() {
    return rand()/RAND_MAX;
}