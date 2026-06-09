#include <stdlib.h>
#include <math.h>

#include "../common/common.h"
#include "../random/random.h"
#include "../result/result.h"

#include "activator.h"
#include "bias.h"
#include "vector.h"
#include "loss.h"

const float eps = 1e-7f;

static Result* equalMseLossFunc(Vector *predict, Vector *target);

static Result* softmaxCelLossFunc(Vector *predict, Vector *target);

static Result* equalMseGradientFunc(Vector *predict, Vector *target);

static Result* softmaxCelGradientFunc(Vector *predict, Vector *target);

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

static Result* equalMseLossFunc(Vector *predict, Vector *target) {
    if (predict == NULL || target == NULL) {
        char *message = "predict or expect vector instance is null for mse loss calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (predict->count != target->count) {
        char *message = "predict and expect vector does not match for mse loss calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    float sum = 0.0;
    for (int i=0;i<predict->count;++i) {
        float difference = predict->getValue(predict, i) - target->getValue(target, i);
        sum += difference*difference;
    }
    sum /= (predict->count*2);
    return createResultWithValue(SUCCESS, NULL, sum);
}

static Result* softmaxCelLossFunc(Vector *predict, Vector *target) {
    if (predict == NULL || target == NULL) {
        char *message = "predict or expect vector instance is null for mse loss calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (predict->count != target->count) {
        char *message = "predict and expect vector does not match for mse loss calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    float sum = 0.0;
    for (int i=0;i<predict->count;++i) {
        float predictValue = predict->getValue(predict, i);
        float targetValue = target->getValue(target, i);
        predictValue = fmaxf(predictValue, eps);
        sum -= log(predictValue)*targetValue;
    }
    return createResultWithValue(SUCCESS, NULL, sum);
}

static Result* equalMseGradientFunc(Vector *predict, Vector *target) {
    if (predict == NULL || target == NULL) {
        char *message = "predict or target vector instance is null for equalMse gradient calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (predict->count != target->count) {
        char *message = "predict and target vector does not match for equalMse gradient calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Vector *gradient = createVector(predict->count);
    if (gradient == NULL) {
        char *message = "can not create vector instance for equalMse gradient calculationr^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<predict->count;++i) {
        float predictValue = predict->getValue(predict, i);
        float targetValue = target->getValue(target, i);
        float gradientValue = predictValue - targetValue;
        gradient->setValue(gradient, i, gradientValue);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, gradient);
}

static Result* softmaxCelGradientFunc(Vector *predict, Vector *target) {
    if (predict == NULL || target == NULL) {
        char *message = "predict or target vector instance is null for softmaxCel gradient calculation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (predict->count != target->count) {
        char *message = "predict and target vector does not match for softmaxCel gradient calculation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Vector *gradient = createVector(predict->count);
    if (gradient == NULL) {
        char *message = "can not create vector instance for softmaxCel gradient calculationr^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<predict->count;++i) {
        float predictValue = predict->getValue(predict, i);
        float targetValue = target->getValue(target, i);
        float gradientValue = predictValue - targetValue;
        gradient->setValue(gradient, i, gradientValue);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, gradient);
}
