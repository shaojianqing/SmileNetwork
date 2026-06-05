#include <stdlib.h>
#include <time.h>

void initRandomSeed() {
    srand(time(NULL));
}

float biasGenerator() {
    return (float)rand()/((float)RAND_MAX + 1.0f);
}

float matrixGenerator() {
    return (float)rand()/((float)RAND_MAX + 1.0f);
}