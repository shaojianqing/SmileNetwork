#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../except/exception.h"
#include "../except/assertion.h"

#include "bias.h"
#include "vector.h"

struct Bias {

    float *elements;

    int count;
};

static Exception MemoryAllocException = {MemoryAllocExceptionType};

Bias* createBias(int dimensionCount, Random random) {
    Bias *bias = (Bias*)allocate(sizeof(Bias));
    if (bias != NULL) {
        bias->elements = (float *)allocate(sizeof(float)*dimensionCount);
        bias->count = dimensionCount;

        if (random != NULL) {
            for (int i=0;i<dimensionCount;++i) {
                bias->elements[i] = random();
            }
        }
    } else {
        throw(&MemoryAllocException, "can not allocate memory for bias creation.");
    }
    return bias;
}

int getBiasElementCount(Bias *this) {
    if (this != NULL) {
        return this->count;
    }
    return 0;
}

void releaseBias(Bias *this) {
    if (this != NULL) {
        release(this->elements);
        release(this);
    }
}

void copyBias(Bias *this, Vector *target) {

    assertNotNull(this, "bias instance is null for copy from vector operation!");
    assertNotNull(target, "vector instance is null for copy from vector operation!");

    assertDataMatch(this->count, getElementCount(target), "bias and vector does not match for copy from vector operation!");

    for (int i=0;i<this->count;++i) {
        float vectorValue = getVectorValue(target, i);
        setBiasValue(this, i, vectorValue);
    }
}

void subBias(Bias *this, Bias *target) {
    
    assertNotNull(this, "bias instance is null for copy from vector operation!");
    assertNotNull(target, "bias instance is null for copy from vector operation!");

    assertDataMatch(this->count, target->count, "bias does not match for addition operation!");

    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float targetValue = target->elements[i];
        this->elements[i] = thisValue - targetValue;
    }
}

void mulBiasNumber(Bias *this, float number) {

    assertNotNull(this, "bias instance is null for number multiplication operation!");

    for (int i=0;i<this->count;++i) {
        float value = this->elements[i];
        this->elements[i] = value * number;
    }
}

float getBiasValue(Bias *this, int index) {
    if (index < this->count) {
        return this->elements[index];
    }
    return 0.0;
}

void setBiasValue(Bias *this, int index, float value) {
    if (index < this->count) {
        this->elements[index] = value;
    }
}
