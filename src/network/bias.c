#include <memory.h>
#include <stdlib.h>

#include "bias.h"

static float getValue(Bias *this, int index);

static void setValue(Bias *this, int index, float value);

Bias *createBias(int dimensionCount) {
    Bias *bias = (Bias*)malloc(sizeof(Bias));
    if (bias != NULL) {
        bias->elements = (float *)malloc(sizeof(float)*dimensionCount);
        bias->count = dimensionCount;

        bias->getValue = getValue;
        bias->setValue = setValue;
    }
    return bias;
}

void releaseBias(Bias *this) {
    if (this != NULL) {
        free(this->elements);
        free(this);
    }
}

static float getValue(Bias *this, int index) {
    if (index<this->count) {
        return this->elements[index];
    }
    return 0.0;
}

static void setValue(Bias *this, int index, float value) {
    if (index<this->count) {
        this->elements[index] = value;
    }
}