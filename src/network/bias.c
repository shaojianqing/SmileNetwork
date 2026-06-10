#include <stdlib.h>

#include "../common/common.h"
#include "../result/result.h"
#include "../memory/memory.h"
#include "../random/random.h"

#include "bias.h"
#include "vector.h"

struct Bias {

    float *elements;

    int count;
};

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

Result* copyBias(Bias *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "bias or vector instance is null for copy operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != getElementCount(vector)) {
        char *message = "bias and vector does not match for copy operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float vectorValue = getVectorValue(vector, i);
        setBiasValue(this, i, vectorValue);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* subBias(Bias *this, Bias *bias) {
    if (this == NULL || bias == NULL) {
        char *message = "bias instance is null for addition operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != bias->count) {
        char *message = "bias does not match for addition operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float biasValue = bias->elements[i];
        this->elements[i] = thisValue - biasValue;
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* mulBiasNumber(Bias *this, float number) {
    if (this == NULL) {
        char *message = "bias instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    for (int i=0;i<this->count;++i) {
        float value = this->elements[i];
        this->elements[i] = value * number;
    }
    return createResultWithoutData(SUCCESS, NULL);
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
