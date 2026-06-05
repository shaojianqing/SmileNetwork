#include <stdlib.h>
#include <math.h>

#include "../common/common.h"
#include "../random/random.h"
#include "../result/result.h"

#include "activator.h"
#include "bias.h"
#include "vector.h"
#include "loss.h"

static Result* equalMseLossFunc(Vector *source, Vector *target);

static Result* softmaxCelLossFunc(Vector *source, Vector *target);

static Result* equalMseGradientFunc(Vector *source, Vector *target);

static Result* softmaxCelGradientFunc(Vector *source, Vector *target);

ActivatorLossFunc getActivatorLossFunc(ActivatorLossKind kind) {
    if (kind == EQUAL_MSE) {
        return equalMseLossFunc;
    } else if (kind == SOFTMAX_CEL) {
        return softmaxCelLossFunc;
    }
    return NULL;
}

ActivatorGradientFunc getActivatorGradientFunc(ActivatorLossKind kind) {
    if (kind == EQUAL_MSE) {
        return equalMseGradientFunc;
    } else if (kind == SOFTMAX_CEL) {
        return softmaxCelGradientFunc;
    }
    return NULL;
}

Activator* getActivatorByActivatorLossKind(ActivatorLossKind kind) {
    if (kind == EQUAL_MSE) {
        return getActivator(EQUAL);
    } else if (kind == SOFTMAX_CEL) {
        return getActivator(SOFTMAX);
    }
    return NULL;
}

static Result* equalMseLossFunc(Vector *source, Vector *target) {
    if (source == NULL || target == NULL) {
        char *message = "predict or expect vector instance is null for mse loss calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (source->count != target->count) {
        char *message = "predict and expect vector does not match for mse loss calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Activator *activator = getActivator(EQUAL);
    Vector *activated = activator->activate(source);

    float sum = 0.0;
    int i = 0;
    for (i=0;i<activated->count;++i) {
        float difference = activated->getValue(activated, i) - target->getValue(target, i);
        sum += difference*difference;
    }
    sum /= (activated->count*2);
    return createResultWithValue(SUCCESS, NULL, sum);
}

static Result* softmaxCelLossFunc(Vector *source, Vector *target) {
    if (source == NULL || target == NULL) {
        char *message = "predict or expect vector instance is null for mse loss calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (source->count != target->count) {
        char *message = "predict and expect vector does not match for mse loss calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Activator *activator = getActivator(SOFTMAX);
    Vector *activateVector = activator->activate(source);

    float sum = 0.0;
    int i = 0;
    for (i=0;i<activateVector->count;++i) {
        float activateValue = activateVector->getValue(activateVector, i);
        float targetValue = target->getValue(target, i);
        sum -= log(activateValue)*targetValue;
    }
    return createResultWithValue(SUCCESS, NULL, sum);
}

static Result* equalMseGradientFunc(Vector *source, Vector *target) {
    if (source == NULL || target == NULL) {
        char *message = "source or target vector instance is null for equalMse gradient calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (source->count != target->count) {
        char *message = "source and target vector does not match for equalMse gradient calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Vector *gradient = createVector(source->count);
    if (gradient == NULL) {
        char *message = "can not create vector instance for equalMse gradient calculationr^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    int i = 0;
    for (i=0;i<source->count;++i) {
        float sourceValue = source->getValue(source, i);
        float targetValue = target->getValue(target, i);
        float gradientValue = sourceValue - targetValue;
        gradient->setValue(gradient, i, gradientValue);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, gradient);
}

static Result* softmaxCelGradientFunc(Vector *source, Vector *target) {
    if (source == NULL || target == NULL) {
        char *message = "source or target vector instance is null for softmaxCel gradient calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (source->count != target->count) {
        char *message = "source and target vector does not match for softmaxCel gradient calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Vector *gradient = createVector(source->count);
    if (gradient == NULL) {
        char *message = "can not create vector instance for softmaxCel gradient calculationr^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    int i = 0;
    for (i=0;i<source->count;++i) {
        float sourceValue = source->getValue(source, i);
        float targetValue = target->getValue(target, i);
        float gradientValue = sourceValue - targetValue;
        gradient->setValue(gradient, i, gradientValue);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, gradient);
}