#include <stdlib.h>
#include <math.h>

#include "../common/common.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"

#include "bias.h"
#include "vector.h"
#include "activator.h"
#include "loss.h"

const float eps = 1e-7f;

static float equalMseLossFunc(Vector *predict, Vector *target);

static float softmaxCelLossFunc(Vector *predict, Vector *target);

static Vector* equalMseGradientFunc(Vector *predict, Vector *target);

static Vector* softmaxCelGradientFunc(Vector *predict, Vector *target);

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

static float equalMseLossFunc(Vector *predict, Vector *target) {

    assertNotNull(predict, "predict vector instance is null for equal mse loss calculation!");
    assertNotNull(target, "target vector instance is null for equal mse loss calculation!");

    assertDataMatch(getElementCount(predict), getElementCount(target), "predict and target vector does not match for mse loss calculation!");
    
    float sum = 0.0;
    int predictCount = getElementCount(predict);
    for (int i=0;i<predictCount;++i) {
        float difference = getVectorValue(predict, i) - getVectorValue(target, i);
        sum += difference*difference;
    }
    sum /= (predictCount*2);
    return sum;
}

static float softmaxCelLossFunc(Vector *predict, Vector *target) {

    assertNotNull(predict, "predict vector instance is null for softmax cel loss calculation!");
    assertNotNull(target, "target vector instance is null for softmax cel loss calculation!");

    assertDataMatch(getElementCount(predict), getElementCount(target), "predict and target vector does not match for softmax cel loss calculation!");

    float sum = 0.0;
    int predictCount = getElementCount(predict);
    for (int i=0;i<predictCount;++i) {
        float predictValue = getVectorValue(predict, i);
        float targetValue = getVectorValue(target, i);
        predictValue = fmaxf(predictValue, eps);
        sum -= log(predictValue)*targetValue;
    }
    return sum;
}

static Vector* equalMseGradientFunc(Vector *predict, Vector *target) {

    assertNotNull(predict, "predict vector instance is null for equal mse gradient calculation!");
    assertNotNull(target, "target vector instance is null for equal mse gradient calculation!");

    assertDataMatch(getElementCount(predict), getElementCount(target), "predict and target vector does not match for equal mse gradient calculation!");

    int predictCount = getElementCount(predict);
    Vector *gradient = createVector(predictCount);
    for (int i=0;i<predictCount;++i) {
        float predictValue = getVectorValue(predict, i);
        float targetValue = getVectorValue(target, i);
        float gradientValue = predictValue - targetValue;
        setVectorValue(gradient, i, gradientValue);
    }
    return gradient;
}

static Vector* softmaxCelGradientFunc(Vector *predict, Vector *target) {

    assertNotNull(predict, "predict vector instance is null for softmax cel gradient calculation!");
    assertNotNull(target, "target vector instance is null for softmax cel gradient calculation!");

    assertDataMatch(getElementCount(predict), getElementCount(target), "predict and target vector does not match for softmax cel gradient calculation!");

    int predictCount = getElementCount(predict);
    Vector *gradient = createVector(predictCount);
    for (int i=0;i<predictCount;++i) {
        float predictValue = getVectorValue(predict, i);
        float targetValue = getVectorValue(target, i);
        float gradientValue = predictValue - targetValue;
        setVectorValue(gradient, i, gradientValue);
    }
    return gradient;
}
