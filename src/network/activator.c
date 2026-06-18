#include <stdlib.h>
#include <math.h>

#include "../common/common.h"
#include "../random/random.h"
#include "../except/exception.h"
#include "../except/assertion.h"

#include "bias.h"
#include "vector.h"
#include "activator.h"

#define RELU_FACTOR            0.01f


static Vector* sigmoidActivate(Vector *this);

static Vector* softmaxActivate(Vector *this);

static Vector* equalActivate(Vector *this);

static Vector* reluActivate(Vector *this);

static Vector* sigmoidDerivative(Vector *this);

static Vector* softmaxDerivative(Vector *this);

static Vector* reluDerivative(Vector *this);

static Vector* equalDerivative(Vector *this);

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

static Vector* sigmoidActivate(Vector *this) {

    assertNotNull(this, "vector instance is null for sigmoid activate operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);
    copyVector(resultVector, this);

    int vectorCount = getElementCount(resultVector);;
    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(resultVector, i);
        value = 1.0/(1.0 + exp(-value));
        setVectorValue(resultVector, i, value);
    }
    return resultVector;
}

static Vector* softmaxActivate(Vector *this) {

    assertNotNull(this, "vector instance is null for softmax activate operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);
    copyVector(resultVector, this);

    int vectorCount = getElementCount(resultVector);
    float maxElement = getVectorValue(resultVector, 0);
    for (int i=0;i<vectorCount;++i) {
        if (getVectorValue(resultVector, i) > maxElement) {
            maxElement = getVectorValue(resultVector, i);
        }
    }

    float sum = 0.0;
    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(resultVector, i);
        sum += exp(value - maxElement);
    }

    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(resultVector, i);
        float item = exp(value - maxElement)/sum;
        setVectorValue(resultVector, i, item);
    }
    return resultVector;
}

static Vector* equalActivate(Vector *this) {

    assertNotNull(this, "vector instance is null for equal activate operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);

    copyVector(resultVector, this);
    return resultVector;
}

static Vector* reluActivate(Vector *this) {
    
    assertNotNull(this, "vector instance is null for relu activate operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);
    for (int i=0;i<thisCount;++i) {
        float thisValue = getVectorValue(this, i);
        if (thisValue > 0.0) {
            setVectorValue(resultVector, i, thisValue);
        } else {
            thisValue = thisValue * RELU_FACTOR;
            setVectorValue(resultVector, i, thisValue);
        }
    }
    return resultVector;
}

static Vector* sigmoidDerivative(Vector *this) {

    assertNotNull(this, "vector instance is null for sigmoid derivative operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);
    copyVector(resultVector, this);

    int vectorCount = getElementCount(resultVector);
    for (int i=0;i<vectorCount;++i) {
        float value = getVectorValue(resultVector, i);
        float result = value * (1.0 - value);
        setVectorValue(resultVector, i, result);
    }
    return resultVector;
}

static Vector* softmaxDerivative(Vector *this) {
    return NULL;
}

static Vector* equalDerivative(Vector *this) {

    assertNotNull(this, "vector instance is null for equal derivative operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);

    int vectorCount = getElementCount(resultVector);
    for (int i=0;i<vectorCount;++i) {
        setVectorValue(resultVector, i, 1.0);
    }
    return resultVector;
}

static Vector* reluDerivative(Vector *this) {
    assertNotNull(this, "vector instance is null for relu derivative operation!");

    int thisCount = getElementCount(this);
    Vector *resultVector = createVector(thisCount);
    copyVector(resultVector, this);

    int vectorCount = getElementCount(resultVector);
    for (int i=0;i<vectorCount;++i) {
        if (getVectorValue(resultVector, i) >= 0.0) {
            setVectorValue(resultVector, i, 1.0);
        } else {
            setVectorValue(resultVector, i, RELU_FACTOR);
        }
    }
    return resultVector;
}