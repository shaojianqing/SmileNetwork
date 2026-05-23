#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../datatype/stringtype.h"

#include "bias.h"
#include "result.h"
#include "vector.h"

static Result* add(Vector *this, Vector *vector);

static Result* mul(Vector *this, Vector *vector);

static Result* addBias(Vector *this, Bias *bias);

static float getValue(Vector *this, int index);

static void setValue(Vector *this, int index, float value);

Vector *createVector(int count) {
    Vector *vector = (Vector*)malloc(sizeof(Vector));
    if (vector != NULL) {
        vector->elements = (float *)malloc(sizeof(float)*count);
        vector->count = count;

        vector->add = add;
        vector->mul = mul;
        vector->getValue = getValue;
        vector->setValue = setValue;
    }
    return vector;
}

static Result* add(Vector *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        String *message = createString("vector instance is null for addition operation^o^");
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        String *message = createString("vector does not match for addition operation^o^");
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
        String *message = createString("vector instance is null for multiplication operation^o^");
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        String *message = createString("vector does not match for multiplication operation^o^");
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

static Result* addBias(Vector *this, Bias *bias) {
    if (this == NULL || bias == NULL) {
        String *message = createString("vector or bias instance is null for multiplication operation^o^");
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != bias->count) {
        String *message = createString("vector or bias does not match for multiplication operation^o^");
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