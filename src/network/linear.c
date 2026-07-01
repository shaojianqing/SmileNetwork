#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../logger/logger.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"
#include "../datatype/stringtype.h"
#include "../json/json.h"

#include "bias.h"
#include "config.h"
#include "vector.h"
#include "activator.h"
#include "matrix.h"
#include "optimizer.h"
#include "loss.h"
#include "linear.h"

struct VectorLayer {

    Bias *modelBias;

    Matrix *modelMatrix;

    Activator *activator;

    Bias *gradientBias;

    Matrix *gradientMatrix;

    VectorLayer *prevLayer;

    VectorLayer *nextLayer;

    Vector *inputVector;

    Vector *resultVector;

    Optimizer optimizer;

    void (*forward)(VectorLayer *this, Vector *vector);

    void (*backward)(VectorLayer *this, Vector *target);

    void (*optimize)(VectorLayer *this, float learnRate);
};

struct InputLayer {

    VectorLayer baseLayer;
};

struct OutputLayer {

    VectorLayer baseLayer;

    ActivatorLossFunc activatorLossFunc;

    ActivatorGradientFunc activatorGradientFunc;
};

struct AffineLayer {

    VectorLayer baseLayer;
};

extern Logger logger;

static void releaseBaselayer(VectorLayer *baseLayer);

static bool prepareBaselayer(VectorLayer *baseLayer, LinearLayerConfig *config);

static void backwardInner(VectorLayer *this, Vector *target);

static void backwardOutput(VectorLayer *this, Vector *target);

static void forwardInner(VectorLayer *this, Vector *vector);

static void forwardOutput(VectorLayer *this, Vector *vector);

static void optimizeInner(VectorLayer *this, float learnRate);

InputLayer* buildInputLayer(LinearLayerConfig *config) {
    InputLayer *inputLayer = (InputLayer*)allocate(sizeof(InputLayer));
    if (inputLayer != NULL) {
        VectorLayer *baseLayer = (VectorLayer*)inputLayer;
        baseLayer->backward = backwardInner;
        baseLayer->optimize = optimizeInner;
        baseLayer->optimizer = SGDOptimizer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseInputLayer(inputLayer);
            return NULL;
        }
    }
    return inputLayer;
}

OutputLayer* buildOutputLayer(LinearLayerConfig *config) {
    OutputLayer *outputLayer = (OutputLayer*)allocate(sizeof(OutputLayer));
    if (outputLayer != NULL) {
        VectorLayer *baseLayer = (VectorLayer*)outputLayer;
        baseLayer->forward = forwardOutput;
        baseLayer->backward = backwardOutput;
        baseLayer->optimize = optimizeInner;
        baseLayer->optimizer = SGDOptimizer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseOutputLayer(outputLayer);
            return NULL;
        }

        if (isLinearOutputLayer(config)) {
            ActivatorLossKind activatorLossKind = getLinearConfigActivatorLossKind(config);
            Activator *activator = getActivatorByActivatorLossKind(activatorLossKind);
            if (activator != NULL) {
                baseLayer->activator = activator;
            } else {
                logger.error("get activator failure when building output layer^o^");
                releaseOutputLayer(outputLayer);
                return NULL;
            }

            ActivatorLossFunc activatorLossFunc = getActivatorLossFunc(activatorLossKind);
            if (activatorLossFunc != NULL) {
                outputLayer->activatorLossFunc = activatorLossFunc;
            } else {
                logger.error("get activator loss func failure when building output layer^o^");
                releaseOutputLayer(outputLayer);
                return NULL;
            }

            ActivatorGradientFunc activatorGradientFunc = getActivatorGradientFunc(activatorLossKind);
            if (activatorGradientFunc != NULL) {
                outputLayer->activatorGradientFunc = activatorGradientFunc;
            } else {
                logger.error("get activator gradient func failure when building output layer^o^");
                releaseOutputLayer(outputLayer);
                return NULL;
            }
        }
    }
    return outputLayer;
}

AffineLayer* buildAffineLayer(LinearLayerConfig *config) {
    AffineLayer *affineLayer = (AffineLayer*)allocate(sizeof(AffineLayer));
    if (affineLayer != NULL) {
        VectorLayer *baseLayer = (VectorLayer*)affineLayer;
        baseLayer->forward = forwardInner;
        baseLayer->backward = backwardInner;
        baseLayer->optimize = optimizeInner;
        baseLayer->optimizer = SGDOptimizer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseAffineLayer(affineLayer);
            return NULL;
        }
    }
    return affineLayer;
}

static bool prepareBaselayer(VectorLayer *baseLayer, LinearLayerConfig *config) {
    int matrixConfigRowCount = getLinearMatrixConfigRowCount(config);
    int matrixConfigColumnCount = getLinearMatrixConfigColumnCount(config);
    baseLayer->modelMatrix = createMatrix(matrixConfigRowCount, matrixConfigColumnCount, matrixGenerator);

    int biasDimensionCount = getLinearBiasConfigDimensionCount(config);
    baseLayer->modelBias = createBias(biasDimensionCount, biasGenerator);

    ActivatorKind activatorKind = getLinearConfigActivatorKind(config);
    Activator *activator = getActivator(activatorKind);
    if (activator != NULL) {
        baseLayer->activator = activator;
    } else {
        logger.error("get activator failure when build base layer^o^");
        return false;
    }
    return true;
}

void releaseInputLayer(InputLayer *inputLayer) {
    if (inputLayer != NULL) {
        VectorLayer *baseLayer = (VectorLayer*)inputLayer;
        releaseBaselayer(baseLayer);
        release(inputLayer);
    }
}

void releaseOutputLayer(OutputLayer *outputLayer) {
    if (outputLayer != NULL) {
        VectorLayer *baseLayer = (VectorLayer*)outputLayer;
        releaseBaselayer(baseLayer);
        release(outputLayer);
    }
}

void releaseAffineLayer(AffineLayer *affineLayer) {
    if (affineLayer != NULL) {
        VectorLayer *baseLayer = (VectorLayer*)affineLayer;
        releaseBaselayer(baseLayer);
        release(affineLayer);
    }
}

static void releaseBaselayer(VectorLayer *baseLayer) {
    Matrix *modelMatrix = baseLayer->modelMatrix;
    Bias *modelBias = baseLayer->modelBias;

    Matrix *gradientMatrix = baseLayer->gradientMatrix;
    Bias *gradientBias = baseLayer->gradientBias;

    releaseMatrix(modelMatrix);
    releaseMatrix(gradientMatrix);

    releaseBias(modelBias);
    releaseBias(gradientBias);
}

void forward(VectorLayer *this, Vector *vector) {
    this->forward(this, vector);
}

void backward(VectorLayer *this, Vector *target) {
    this->backward(this, target);
}

void optimize(VectorLayer *this, float learnRate) {
    this->optimize(this, learnRate);
}

void input(InputLayer *this, Vector *vector) {
    forwardInner((VectorLayer*)this, vector);
}

Vector* output(OutputLayer *this) {
    VectorLayer *baseLayer = (VectorLayer*)this;
    return baseLayer->resultVector;
}

float loss(OutputLayer *this, Vector *expect) {

    assertNotNull(this->activatorLossFunc, "loss function not configured for loss calculation!");

    VectorLayer *baseLayer = (VectorLayer*)this;
    return this->activatorLossFunc(baseLayer->resultVector, expect);
}

void setNextLayer(VectorLayer *this, VectorLayer *next) {
    this->nextLayer = next;
}

void setPrevLayer(VectorLayer *this, VectorLayer *prev) {
    this->prevLayer = prev;
}

static void forwardInner(VectorLayer *this, Vector *inputVector) {
    Activator *activator = this->activator;
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;

    this->inputVector = inputVector;
    Vector *innerVector = mulVector(matrix, inputVector);
    addBias(innerVector, bias);

    this->resultVector = activator->activate(innerVector);
    releaseVector(innerVector);
    
    if (this->nextLayer != NULL) {
        VectorLayer *nextLayer = this->nextLayer;
        nextLayer->forward(nextLayer, this->resultVector);
    }
}

static void forwardOutput(VectorLayer *this, Vector *inputVector) {
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;
    
    this->inputVector = inputVector;
    Vector *innerVector = mulVector(matrix, inputVector);
    addBias(innerVector, bias);

    Activator *activator = this->activator;
    this->resultVector = activator->activate(innerVector);
    releaseVector(innerVector);
}
    
static void backwardInner(VectorLayer *this, Vector *prevGradientVector) {

    assertNotNull(this->activator, "activator instance not configured for gradient calculation!");

    Vector *thisGradientVector = this->activator->derivative(this->resultVector);
    Vector *gradientVector = mulHadamard(thisGradientVector, prevGradientVector);

    releaseVector(thisGradientVector);
    releaseVector(prevGradientVector);
    
    Vector *inputVector = this->inputVector;
    this->gradientMatrix = mulTensor(gradientVector, this->inputVector);

    int biasCount = getElementCount(gradientVector);
    Bias *gradientBias = createBias(biasCount, NULL);
    copyBias(gradientBias, gradientVector);

    this->gradientBias = gradientBias;
    
    releaseVector(this->resultVector);
    this->resultVector = NULL;

    VectorLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Vector *prevGradientVector = mulMatrixVector(gradientVector, modelMatrix);
        prevLayer->backward(prevLayer, prevGradientVector);
    }
    releaseVector(gradientVector);
}

static void backwardOutput(VectorLayer *this, Vector *target) {
    
    OutputLayer *outputLayer = (OutputLayer*)this;

    assertNotNull(outputLayer->activatorGradientFunc, "gradient func not configured for gradient calculation!");

    Vector *inputVector = this->inputVector;
    Vector *gradientVector = outputLayer->activatorGradientFunc(this->resultVector, target);
    this->gradientMatrix = mulTensor(gradientVector, inputVector);

    int biasCount = getElementCount(gradientVector);
    Bias *gradientBias = createBias(biasCount, NULL);
    copyBias(gradientBias, gradientVector);

    this->gradientBias = gradientBias;
    
    releaseVector(this->resultVector);
    this->resultVector = NULL;

    VectorLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Vector *prevGradientVector = mulMatrixVector(gradientVector, modelMatrix);
        prevLayer->backward(prevLayer, prevGradientVector);
    } 
    releaseVector(gradientVector); 
}

static void optimizeInner(VectorLayer *this, float learnRate) {
    Bias *modelBias = this->modelBias;
    Matrix *modelMatrix = this->modelMatrix;

    Bias *gradientBias = this->gradientBias;
    Matrix *gradientMatrix = this->gradientMatrix;

    this->optimizer(modelMatrix, modelBias, gradientMatrix, gradientBias, learnRate);
    
    releaseBias(this->gradientBias);
    releaseMatrix(this->gradientMatrix);

    this->gradientBias = NULL;
    this->gradientMatrix = NULL;

    VectorLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        prevLayer->optimize(prevLayer, learnRate);
    }
}
