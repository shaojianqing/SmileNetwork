#include <stdlib.h>
#include <math.h>

#include "../common/common.h"
#include "../random/random.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"

#include "bias.h"
#include "vector.h"
#include "activator.h"

#define RELU_FACTOR            0.01f


static Result* sigmoidActivate(Vector *vector);

static Result* softmaxActivate(Vector *vector);

static Result* equalActivate(Vector *vector);

static Result* reluActivate(Vector *vector);

static Result* sigmoidDerivative(Vector *vector);

static Result* softmaxDerivative(Vector *vector);

static Result* reluDerivative(Vector *vector);

static Result* equalDerivative(Vector *vector);

static Activator sigmoidActivator;

static Activator softmaxActivator;

static Activator equalActivator;

static Activator reluActivator;

void initActivatorMap() {
    sigmoidActivator.activate = sigmoidActivate;
    sigmoidActivator.derivative = sigmoidDerivative;

    softmaxActivator.activate = softmaxActivate;
    softmaxActivator.derivative = softmaxDerivative;

    equalActivator.activate = equalActivate;
    equalActivator.derivative = equalDerivative;

    reluActivator.activate = reluActivate;
    reluActivator.derivative = reluDerivative;
}

Activator* getActivator(ActivatorKind kind) {
    if (kind == SIGMOID) {
        return &sigmoidActivator;
    } else if (kind == SOFTMAX) {
        return &softmaxActivator;
    } else if (kind == EQUAL) {
        return &equalActivator;
    } else if (kind == RELU) {
        return &reluActivator;
    }
    return NULL;
}

static Result* sigmoidActivate(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for equal activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    Result *copyResult = copyVector(vector, this);
    if (!success(copyResult)) {
        return copyResult;
    }
    releaseResult(copyResult);

    int vectorCount = getElementCount(vector);;
    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(vector, i);
        value = 1.0/(1.0 + exp(-value));
        setVectorValue(vector, i, value);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}

static Result* softmaxActivate(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for equal activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    Result *copyResult = copyVector(vector, this);
    if (!success(copyResult)) {
        return copyResult;
    }
    releaseResult(copyResult);

    int vectorCount = getElementCount(vector);
    float maxElement = getVectorValue(vector, 0);
    for (int i=0;i<vectorCount;++i) {
        if (getVectorValue(vector, i) > maxElement) {
            maxElement = getVectorValue(vector, i);
        }
    }

    float sum = 0.0;
    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(vector, i);
        sum += exp(value - maxElement);
    }

    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(vector, i);
        float item = exp(value - maxElement)/sum;
        setVectorValue(vector, i, item);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}

static Result* equalActivate(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for equal activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    Result *copyResult = copyVector(vector, this);
    if (!success(copyResult)) {
        return copyResult;
    }
    releaseResult(copyResult);
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}

static Result* reluActivate(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for relu activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<thisCount;++i) {
        float thisValue = getVectorValue(this, i);
        if (thisValue > 0.0) {
            setVectorValue(vector, i, thisValue);
        } else {
            thisValue = thisValue * RELU_FACTOR;
            setVectorValue(vector, i, thisValue);
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}

static Result* sigmoidDerivative(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for relu activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    Result *copyResult = copyVector(vector, this);
    if (!success(copyResult)) {
        return copyResult;
    }
    releaseResult(copyResult);

    int vectorCount = getElementCount(vector);
    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(vector, i);
        float result = value * (1.0 - value);
        setVectorValue(vector, i, result);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}

static Result* softmaxDerivative(Vector *this) {
    return NULL;
}

static Result* equalDerivative(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for relu activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    int vectorCount = getElementCount(vector);
    for (int i=0;i<vectorCount;++i) {
        setVectorValue(vector, i, 1.0);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}

static Result* reluDerivative(Vector *this) {
    if (this == NULL) {
        char *message = "vector instance is null for relu activation operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    int thisCount = getElementCount(this);
    Vector *vector = createVector(thisCount);
    if (vector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    Result *copyResult = copyVector(vector, this);
    if (!success(copyResult)) {
        return copyResult;
    }
    releaseResult(copyResult);

    int vectorCount = getElementCount(vector);
    for (int i=0;i<vectorCount;++i) {
        if (getVectorValue(vector, i) >= 0.0) {
            setVectorValue(vector, i, 1.0);
        } else {
            setVectorValue(vector, i, RELU_FACTOR);
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, vector);
}