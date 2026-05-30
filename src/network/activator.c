#include <stdlib.h>
#include <math.h>

#include "../common/common.h"
#include "../random/random.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"

#include "bias.h"
#include "vector.h"
#include "activator.h"

static Vector* sigmoidActivate(Vector *vector);

static Vector* softmaxActivate(Vector *vector);

static Vector* equalActivate(Vector *vector);

static Vector* reluActivate(Vector *vector);

static Vector* sigmoidDerivative(Vector *vector);

static Vector* softmaxDerivative(Vector *vector);

static Vector* reluDerivative(Vector *vector);

static Vector* equalDerivative(Vector *vector);

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

static Vector* sigmoidActivate(Vector *vector) {
    if (vector != NULL) {
        int i = 0;
        for (i=0;i<vector->count;++i) {
            float value = vector->getValue(vector, i);
            value = 1.0/(1.0 + exp(-value));
            vector->setValue(vector, i, value);
        }
    }
    return vector;
}

static Vector* softmaxActivate(Vector *vector) {
    if (vector != NULL) {
        int i = 0;
        float sum = 0.0;
        for (i=0;i<vector->count;++i) {
            float value = vector->getValue(vector, i);
            sum += exp(value);
        }

        for (i=0;i<vector->count;++i) {
            float value = vector->getValue(vector, i);
            float item = exp(value)/sum;
            vector->setValue(vector, i, item);
        }
    }
    return vector;
}

static Vector* equalActivate(Vector *vector) {
    return vector;
}

static Vector* reluActivate(Vector *vector) {
    if (vector != NULL) {
        int i = 0;
        for (i=0;i<vector->count;++i) {
            if (vector->getValue(vector, i) < 0.0) {
                vector->setValue(vector, i, 0);
            }
        }
    }
    return vector;
}

static Vector* sigmoidDerivative(Vector *vector) {
    vector = sigmoidActivate(vector);
    if (vector != NULL) {
        int i = 0;
        for (i=0;i<vector->count;++i) {
            float value = vector->getValue(vector, i);
            float result = value * (1.0 - value);
            vector->setValue(vector, i, result);
        }
    }
    return vector;
}

static Vector* softmaxDerivative(Vector *vector) {
    return NULL;
}

static Vector* equalDerivative(Vector *vector) {
    if (vector != NULL) {
        int i = 0;
        for (i=0;i<vector->count;++i) {
            vector->setValue(vector, i, 1.0);
        }
    }
    return vector;
}

static Vector* reluDerivative(Vector *vector) {
    if (vector != NULL) {
        int i = 0;
        for (i=0;i<vector->count;++i) {
            if (vector->getValue(vector, i) >= 0.0) {
                vector->setValue(vector, i, 1.0);
            } else {
                vector->setValue(vector, i, 0.0);
            }
        }
    }
    return vector;
}