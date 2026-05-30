#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../result/result.h"

#include "bias.h"
#include "vector.h"
#include "matrix.h"

static Result* add(Vector *this, Vector *vector);

static Result* mul(Vector *this, Vector *vector);

static Result* addBias(Vector *this, Bias *bias);

static Result* copy(Vector *this, Vector *target);

static Result* matrixMul(Vector *this, Vector *target);

static float getValue(Vector *this, int index);

static void setValue(Vector *this, int index, float value);

Vector *createVector(int count) {
    Vector *vector = (Vector*)allocate(sizeof(Vector));
    if (vector != NULL) {
        vector->elements = (float *)allocate(sizeof(float)*count);
        vector->count = count;

        vector->add = add;
        vector->mul = mul;
        vector->addBias = addBias;

        vector->copy = copy;
        vector->getValue = getValue;
        vector->setValue = setValue;
    }
    return vector;
}

static Result* add(Vector *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "vector instance is null for addition operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        char *message = "vector does not match for addition operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    int i = 0;
    for (i=0;i<this->count;++i) {
        float thisValue = this->getValue(this, i);
        float vectorValue = vector->getValue(vector, i);

        float totalValue = thisValue + vectorValue;
        this->setValue(this, i, totalValue);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* mul(Vector *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "vector instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        char *message = "vector does not match for multiplication operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    int i = 0;
    float sum = 0.0;
    for (i=0;i<this->count;++i) {
        float thisValue = this->getValue(this, i);
        float vectorValue = vector->getValue(vector, i);
        sum += thisValue * vectorValue;
    }
    return createResultWithValue(SUCCESS, NULL, sum);
}

static Result* matrixMul(Vector *this, Vector *target) {
    if (this == NULL || target == NULL) {
        char *message = "vector instance is null for matrix multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    Matrix *resultMatrix = createMatrix(this->count, target->count, NULL);
    if (resultMatrix == NULL) {
        char *message = "can not create matrix instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOCATE_ERROR, message);
    }

    int i = 0, j = 0;
    for (i=0;i<this->count;++i) {
        for (j=0;j<target->count;++j) {
            float thisValue = this->getValue(this, i);
            float targetValue = target->getValue(target, j);
            float resultValue = thisValue*targetValue;
            resultMatrix->setValue(resultMatrix, i, j, resultValue);
        }
    }

    return createResultWithData(SUCCESS, NULL, TYPE_METRIX, resultMatrix);
}

static Result* addBias(Vector *this, Bias *bias) {
    if (this == NULL || bias == NULL) {
        char *message = "vector or bias instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != bias->count) {
        char *message = "vector and bias does not match for multiplication operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    int i = 0;
    for (i=0;i<this->count;++i) {
        float thisValue = this->getValue(this, i);
        float biasValue = bias->getValue(bias, i);

        float totalValue = thisValue + biasValue;
        this->setValue(this, i, totalValue);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* copy(Vector *this, Vector *target) {
    if (this == NULL || target == NULL) {
        char *message = "vector instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != target->count) {
        char *message = "vector does not match for multiplication operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    int i = 0;
    for (i=0;i<this->count;++i) {
        float thisValue = this->getValue(this, i);
        float targetValue = target->getValue(target, i);

        this->setValue(this, i, targetValue);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static float getValue(Vector *this, int index) {
    if (index<this->count) {
        return this->elements[index];
    }
    return 0.0;
}

static void setValue(Vector *this, int index, float value) {
    if (index<this->count) {
        this->elements[index] = value;
    }
}