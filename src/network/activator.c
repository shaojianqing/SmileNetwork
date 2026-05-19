#include <stdlib.h>

#include "../common/common.h"

#include "result.h"
#include "vector.h"
#include "activator.h"

static Vector* sigmoid(Vector *vector);

static Vector* softmax(Vector *vector);

static Vector* relu(Vector *vector);

Activator getActivator(ActivatorKind kind) {
    if (kind == SIGMONID) {
        return sigmoid;
    } else if (kind == SOFTMAX) {
        return softmax;
    } else if (kind == RELU) {
        return relu;
    }
    return NULL;
}

static Vector* sigmoid(Vector *vector) {
    return NULL;
}

static Vector* softmax(Vector *vector) {
    return NULL;
}

static Vector* relu(Vector *vector) {
    return NULL;
}