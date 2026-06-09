#include <stdlib.h>

#include "../common/common.h"
#include "../result/result.h"
#include "../memory/memory.h"
#include "../random/random.h"

#include "bias.h"
#include "vector.h"

static float getValue(Bias *this, int index);

static void setValue(Bias *this, int index, float value);

static Result* copy(Bias *this, Vector *vector);

static Result* subBias(Bias *this, Bias *bias);

static Result* mulNumber(Bias *this, float number);

Bias *createBias(int dimensionCount, Random random) {
    Bias *bias = (Bias*)allocate(sizeof(Bias));
    if (bias != NULL) {
        bias->getValue = getValue;
        bias->setValue = setValue;
        bias->copy = copy;
        bias->subBias = subBias;
        bias->mulNumber = mulNumber;

        bias->count = dimensionCount;
        bias->elements = (float *)allocate(sizeof(float)*dimensionCount);

        if (random != NULL) {
            for (int i=0;i<dimensionCount;++i) {
                bias->elements[i] = random();
            }
        }
    }
    return bias;
}

void releaseBias(Bias *this) {
    if (this != NULL) {
        release(this->elements);
        release(this);
    }
}

static Result* copy(Bias *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "bias or vector instance is null for copy operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        char *message = "bias and vector does not match for copy operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float vectorValue = vector->getValue(vector, i);
        this->setValue(this, i, vectorValue);
    }

    return createResultWithoutData(SUCCESS, NULL);
}

static Result* subBias(Bias *this, Bias *bias) {
    if (this == NULL || bias == NULL) {
        char *message = "bias instance is null for addition operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != bias->count) {
        char *message = "bias does not match for addition operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float thisValue = this->getValue(this, i);
        float biasValue = bias->getValue(bias, i);

        float resultValue = thisValue - biasValue;
        this->setValue(this, i, resultValue);
    }

    return createResultWithoutData(SUCCESS, NULL);
}

static Result* mulNumber(Bias *this, float number) {
    if (this == NULL) {
        char *message = "bias instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    for (int i=0;i<this->count;++i) {
        float value = this->getValue(this, i);
        this->setValue(this, i, value * number);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static float getValue(Bias *this, int index) {
    if (index < this->count) {
        return this->elements[index];
    }
    return 0.0;
}

static void setValue(Bias *this, int index, float value) {
    if (index < this->count) {
        this->elements[index] = value;
    }
}