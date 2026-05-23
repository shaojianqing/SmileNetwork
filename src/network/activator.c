#include <stdlib.h>

#include "../common/common.h"
#include "../datatype/stringtype.h"

#include "bias.h"
#include "result.h"
#include "vector.h"
#include "activator.h"

static Vector* sigmoidActivate(Vector *vector);

static Vector* softmaxActivate(Vector *vector);

static Vector* reluActivate(Vector *vector);

static Vector* sigmoidDerivative(Vector *vector);

static Vector* softmaxDerivative(Vector *vector);

static Vector* reluDerivative(Vector *vector);

static Activator sigmoidActivator;
static Activator softmaxActivator;
static Activator reluActivator;

void initActivatorMap() {
    sigmoidActivator.activate = sigmoidActivate;
    sigmoidActivator.derivative = sigmoidDerivative;

    softmaxActivator.activate = softmaxActivate;
    softmaxActivator.derivative = softmaxDerivative;

    reluActivator.activate = reluActivate;
    reluActivator.derivative = reluDerivative;
}

Activator* getActivator(ActivatorKind kind) {
    if (kind == SIGMONID) {
        return &sigmoidActivator;
    } else if (kind == SOFTMAX) {
        return &softmaxActivator;
    } else if (kind == RELU) {
        return &reluActivator;
    }
    return NULL;
}

static Vector* sigmoidActivate(Vector *vector) {
    return NULL;
}

static Vector* softmaxActivate(Vector *vector) {
    return NULL;
}

static Vector* reluActivate(Vector *vector) {
    return NULL;
}

static Vector* sigmoidDerivative(Vector *vector) {
    return NULL;
}

static Vector* softmaxDerivative(Vector *vector) {
    return NULL;
}

static Vector* reluDerivative(Vector *vector) {
    return NULL;
}