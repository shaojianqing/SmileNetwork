#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../logger/logger.h"
#include "../except/exception.h"
#include "../except/assertion.h"

#include "bias.h"
#include "config.h"
#include "vector.h"
#include "activator.h"
#include "matrix.h"
#include "optimizer.h"
#include "loss.h"
#include "layer.h"

struct BaseLayer {

    Bias *modelBias;

    Matrix *modelMatrix;

    Activator *activator;

    Bias *gradientBias;

    Matrix *gradientMatrix;

    BaseLayer *prevLayer;

    BaseLayer *nextLayer;

    Vector *inputVector;

    Vector *resultVector;

    Optimizer optimizer;

    void (*forward)(BaseLayer *this, Vector *vector);

    void (*backward)(BaseLayer *this, Vector *target);

    void (*optimize)(BaseLayer *this, float learnRate);
};

struct InputLayer {

    BaseLayer baseLayer;
};

struct OutputLayer {

    BaseLayer baseLayer;

    ActivatorLossFunc activatorLossFunc;

    ActivatorGradientFunc activatorGradientFunc;
};

struct LinearLayer {

    BaseLayer baseLayer;
};

extern Logger logger;

static void releaseBaselayer(BaseLayer *baseLayer);

static bool prepareBaselayer(BaseLayer *baseLayer, LayerConfig *config);

static void backwardInner(BaseLayer *this, Vector *target);

static void backwardOutput(BaseLayer *this, Vector *target);

static void forwardInner(BaseLayer *this, Vector *vector);

static void forwardOutput(BaseLayer *this, Vector *vector);

static void optimizeInner(BaseLayer *this, float learnRate);

InputLayer* buildInputLayer(LayerConfig *config) {
    InputLayer *inputLayer = (InputLayer*)allocate(sizeof(InputLayer));
    if (inputLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)inputLayer;
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

OutputLayer* buildOutputLayer(LayerConfig *config) {
    OutputLayer *outputLayer = (OutputLayer*)allocate(sizeof(OutputLayer));
    if (outputLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)outputLayer;
        baseLayer->forward = forwardOutput;
        baseLayer->backward = backwardOutput;
        baseLayer->optimize = optimizeInner;
        baseLayer->optimizer = SGDOptimizer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseOutputLayer(outputLayer);
            return NULL;
        }

        if (isOutputLayer(config)) {
            ActivatorLossKind activatorLossKind = getConfigActivatorLossKind(config);
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

LinearLayer* buildLinearLayer(LayerConfig *config) {
    LinearLayer *linearLayer = (LinearLayer*)allocate(sizeof(LinearLayer));
    if (linearLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)linearLayer;
        baseLayer->forward = forwardInner;
        baseLayer->backward = backwardInner;
        baseLayer->optimize = optimizeInner;
        baseLayer->optimizer = SGDOptimizer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseLinearLayer(linearLayer);
            return NULL;
        }
    }
    return linearLayer;
}

static bool prepareBaselayer(BaseLayer *baseLayer, LayerConfig *config) {
    int matrixConfigRowCount = getMatrixConfigRowCount(config);
    int matrixConfigColumnCount = getMatrixConfigColumnCount(config);
    Matrix *modelMatrix = createMatrix(matrixConfigRowCount, matrixConfigColumnCount, matrixGenerator);
    if (modelMatrix != NULL) {
        baseLayer->modelMatrix = modelMatrix;
    } else {
        logger.error("create model metrix failure when build base layer for memory allocation error^o^");
        return false;
    }

    int biasDimensionCount = getBiasConfigDimensionCount(config);
    Bias *modelBias = createBias(biasDimensionCount, biasGenerator);
    if (modelBias != NULL) {
        baseLayer->modelBias = modelBias;
    } else {
        logger.error("create model bias failure when build base layer for memory allocation error^o^");
        return false;
    }

    ActivatorKind activatorKind = getConfigActivatorKind(config);
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
        BaseLayer *baseLayer = (BaseLayer*)inputLayer;
        releaseBaselayer(baseLayer);
        release(inputLayer);
    }
}

void releaseOutputLayer(OutputLayer *outputLayer) {
    if (outputLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)outputLayer;
        releaseBaselayer(baseLayer);
        release(outputLayer);
    }
}

void releaseLinearLayer(LinearLayer *linearLayer) {
    if (linearLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)linearLayer;
        releaseBaselayer(baseLayer);
        release(linearLayer);
    }
}

static void releaseBaselayer(BaseLayer *baseLayer) {
    Matrix *modelMatrix = baseLayer->modelMatrix;
    Bias *modelBias = baseLayer->modelBias;

    Matrix *gradientMatrix = baseLayer->gradientMatrix;
    Bias *gradientBias = baseLayer->gradientBias;

    releaseMatrix(modelMatrix);
    releaseMatrix(gradientMatrix);

    releaseBias(modelBias);
    releaseBias(gradientBias);
}

void forward(BaseLayer *this, Vector *vector) {
    this->forward(this, vector);
}

void backward(BaseLayer *this, Vector *target) {
    this->backward(this, target);
}

void optimize(BaseLayer *this, float learnRate) {
    this->optimize(this, learnRate);
}

void input(InputLayer *this, Vector *vector) {
    forwardInner((BaseLayer*)this, vector);
}

Vector* output(OutputLayer *this) {
    BaseLayer *baseLayer = (BaseLayer*)this;
    return baseLayer->resultVector;
}

float loss(OutputLayer *this, Vector *expect) {

    assertNotNull(this->activatorLossFunc, "loss function not configured for loss calculation!");

    BaseLayer *baseLayer = (BaseLayer*)this;
    return this->activatorLossFunc(baseLayer->resultVector, expect);
}

void setNextLayer(BaseLayer *this, BaseLayer *next) {
    this->nextLayer = next;
}

void setPrevLayer(BaseLayer *this, BaseLayer *prev) {
    this->prevLayer = prev;
}

static void forwardInner(BaseLayer *this, Vector *inputVector) {
    Activator *activator = this->activator;
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;

    this->inputVector = inputVector;
    Vector *innerVector = mulVector(matrix, inputVector);
    addBias(innerVector, bias);

    this->resultVector = activator->activate(innerVector);
    releaseVector(innerVector);
    
    if (this->nextLayer != NULL) {
        BaseLayer *nextLayer = this->nextLayer;
        nextLayer->forward(nextLayer, this->resultVector);
    }
}

static void forwardOutput(BaseLayer *this, Vector *inputVector) {
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;
    
    this->inputVector = inputVector;
    Vector *innerVector = mulVector(matrix, inputVector);
    addBias(innerVector, bias);

    Activator *activator = this->activator;
    this->resultVector = activator->activate(innerVector);
    releaseVector(innerVector);
}
    
static void backwardInner(BaseLayer *this, Vector *prevGradientVector) {

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

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Vector *prevGradientVector = mulMatrixVector(gradientVector, modelMatrix);
        prevLayer->backward(prevLayer, prevGradientVector);
    }
    releaseVector(gradientVector);
}

static void backwardOutput(BaseLayer *this, Vector *target) {
    
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

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Vector *prevGradientVector = mulMatrixVector(gradientVector, modelMatrix);
        prevLayer->backward(prevLayer, prevGradientVector);
    } 
    releaseVector(gradientVector); 
}

static void optimizeInner(BaseLayer *this, float learnRate) {
    Bias *modelBias = this->modelBias;
    Matrix *modelMatrix = this->modelMatrix;

    Bias *gradientBias = this->gradientBias;
    Matrix *gradientMatrix = this->gradientMatrix;

    this->optimizer(modelMatrix, modelBias, gradientMatrix, gradientBias, learnRate);
    
    releaseBias(this->gradientBias);
    releaseMatrix(this->gradientMatrix);

    this->gradientBias = NULL;
    this->gradientMatrix = NULL;

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        prevLayer->optimize(prevLayer, learnRate);
    }
}
