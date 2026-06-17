#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../logger/logger.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"

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

    Result* (*forward)(BaseLayer *this, Vector *vector);

    Result* (*backward)(BaseLayer *this, Vector *target);

    Result* (*optimize)(BaseLayer *this, float learnRate);
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

static Result* backwardInner(BaseLayer *this, Vector *target);

static Result* backwardOutput(BaseLayer *this, Vector *target);

static Result* forwardInner(BaseLayer *this, Vector *vector);

static Result* forwardOutput(BaseLayer *this, Vector *vector);

static Result* optimizeInner(BaseLayer *this, float learnRate);

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

Result* forward(BaseLayer *this, Vector *vector) {
    return this->forward(this, vector);
}

Result* backward(BaseLayer *this, Vector *target) {
    return this->backward(this, target);
}

Result* optimize(BaseLayer *this, float learnRate) {
    return this->optimize(this, learnRate);
}

Result* input(InputLayer *this, Vector *vector) {
    return forwardInner((BaseLayer*)this, vector);
}

Result* output(OutputLayer *this) {
    BaseLayer *baseLayer = (BaseLayer*)this;
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, baseLayer->resultVector);
}

Result* loss(OutputLayer *this, Vector *expect) {
    if (this->activatorLossFunc == NULL) {
        char *message = "loss func not configured for loss calculation^o^";
        return createResultWithoutData(LOSSFUNC_NO_CONFIG, message);
    }

    BaseLayer *baseLayer = (BaseLayer*)this;
    return this->activatorLossFunc(baseLayer->resultVector, expect);
}

void setNextLayer(BaseLayer *this, BaseLayer *next) {
    this->nextLayer = next;
}

void setPrevLayer(BaseLayer *this, BaseLayer *prev) {
    this->prevLayer = prev;
}

static Result* forwardInner(BaseLayer *this, Vector *inputVector) {
    Activator *activator = this->activator;
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;

    this->inputVector = inputVector;
    Result *matrixMulResult = mulVector(matrix, inputVector);
    if (!success(matrixMulResult)) {
        return matrixMulResult;
    }

    Vector *innerVector = (Vector*)getData(matrixMulResult);
    releaseResult(matrixMulResult);
    Result *addBiasResult = addBias(innerVector, bias);
    if (!success(addBiasResult)) {
        return addBiasResult;
    }
    releaseResult(addBiasResult);

    Result *activateResult = activator->activate(innerVector);
    releaseVector(innerVector);

    if (!success(activateResult)) {
        return activateResult;
    }
    this->resultVector = (Vector*)getData(activateResult);
    releaseResult(activateResult);
    
    if (this->nextLayer != NULL) {
        BaseLayer *nextLayer = this->nextLayer;
        return nextLayer->forward(nextLayer, this->resultVector);
    } else {
        return createResultWithoutData(SUCCESS, NULL);
    }
}

static Result* forwardOutput(BaseLayer *this, Vector *inputVector) {
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;
    
    this->inputVector = inputVector;
    Result *matrixMulResult = mulVector(matrix, inputVector);
    if (!success(matrixMulResult)) {
        return matrixMulResult;
    }
    Vector *innerVector = (Vector*)getData(matrixMulResult);
    releaseResult(matrixMulResult);

    Result *addBiasResult = addBias(innerVector, bias);
    if (!success(addBiasResult)) {
        return addBiasResult;
    }
    releaseResult(addBiasResult);

    Activator *activator = this->activator;
    Result *activateResult = activator->activate(innerVector);
    releaseVector(innerVector);

    if (!success(activateResult)) {
        return activateResult;
    }
    this->resultVector = (Vector*)getData(activateResult);
    releaseResult(activateResult);

    return createResultWithoutData(SUCCESS, NULL);
}
    
static Result* backwardInner(BaseLayer *this, Vector *prevGradientVector) {
    if (this->activator == NULL) {
        char *message = "activator instance not configured for gradient calculation^o^";
        return createResultWithoutData(GRADFUNC_NO_CONFIG, message);
    }

    Result *derivativeResult = this->activator->derivative(this->resultVector);
    if (!success(derivativeResult)) {
        return derivativeResult;
    }

    Vector *thisGradientVector = getData(derivativeResult);
    releaseResult(derivativeResult);

    Result *mulHamdResult = mulHadamard(thisGradientVector, prevGradientVector);
    releaseVector(thisGradientVector);
    releaseVector(prevGradientVector);

    if (!success(mulHamdResult)) {
        return mulHamdResult;
    }
    Vector *gradientVector = (Vector *)getData(mulHamdResult);
    releaseResult(mulHamdResult);
    
    Vector *inputVector = this->inputVector;
    Result *matrixMulResult = mulTensor(gradientVector, inputVector);
    if (!success(matrixMulResult)) {
        return matrixMulResult;
    }
    
    this->gradientMatrix = (Matrix*)getData(matrixMulResult);
    releaseResult(matrixMulResult);

    int biasCount = getElementCount(gradientVector);
    Bias *gradientBias = createBias(biasCount, NULL);
    Result *biasCopyResult = copyBias(gradientBias, gradientVector);
    if (!success(biasCopyResult)) {
        return biasCopyResult;
    }
    releaseResult(biasCopyResult);
    this->gradientBias = gradientBias;
    
    releaseVector(this->resultVector);
    this->resultVector = NULL;

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Result *mulVectorResult = mulMatrixVector(gradientVector, modelMatrix);
        releaseVector(gradientVector);
        if (!success(mulVectorResult)) {
            return mulVectorResult;
        }

        Vector *prevGradientVector = (Vector*)getData(mulVectorResult);
        releaseResult(mulVectorResult);

        return prevLayer->backward(prevLayer, prevGradientVector);
    } else {
        releaseVector(gradientVector);
        return createResultWithoutData(SUCCESS, NULL);
    }
}

static Result* backwardOutput(BaseLayer *this, Vector *target) {
    OutputLayer *outputLayer = (OutputLayer*)this;
    if (outputLayer->activatorGradientFunc == NULL) {
        char *message = "gradient func not configured for gradient calculation^o^";
        return createResultWithoutData(GRADFUNC_NO_CONFIG, message);
    }

    Result *gradientResult = outputLayer->activatorGradientFunc(this->resultVector, target);
    if (!success(gradientResult)) {
        return gradientResult;
    }
    
    Vector *gradientVector = (Vector*)getData(gradientResult);
    releaseResult(gradientResult);

    Vector *inputVector = this->inputVector;
    Result *matrixMulResult = mulTensor(gradientVector, inputVector);
    if (!success(matrixMulResult)) {
        return matrixMulResult;
    }
    
    this->gradientMatrix = (Matrix*)getData(matrixMulResult);
    releaseResult(matrixMulResult);

    int biasCount = getElementCount(gradientVector);
    Bias *gradientBias = createBias(biasCount, NULL);
    Result *biasCopyResult = copyBias(gradientBias, gradientVector);
    if (!success(biasCopyResult)) {
        return biasCopyResult;
    }
    releaseResult(biasCopyResult);
    this->gradientBias = gradientBias;
    
    releaseVector(this->resultVector);
    this->resultVector = NULL;

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Result *mulVectorResult = mulMatrixVector(gradientVector, modelMatrix);
        releaseVector(gradientVector);
        if (!success(mulVectorResult)) {
            return mulVectorResult;
        }
        Vector *prevGradientVector = (Vector*)getData(mulVectorResult);
        releaseResult(mulVectorResult);
        
        return prevLayer->backward(prevLayer, prevGradientVector);
    } else {
        releaseVector(gradientVector);
        return createResultWithoutData(SUCCESS, NULL);
    }
}

static Result* optimizeInner(BaseLayer *this, float learnRate) {
    Bias *modelBias = this->modelBias;
    Matrix *modelMatrix = this->modelMatrix;

    Bias *gradientBias = this->gradientBias;
    Matrix *gradientMatrix = this->gradientMatrix;

    Result *result = this->optimizer(modelMatrix, modelBias, gradientMatrix, gradientBias, learnRate);
    if (!success(result)) {
        return result;
    }
    releaseResult(result);
    
    releaseBias(this->gradientBias);
    releaseMatrix(this->gradientMatrix);

    this->gradientBias = NULL;
    this->gradientMatrix = NULL;

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        return prevLayer->optimize(prevLayer, learnRate);
    } else {
        return createResultWithoutData(SUCCESS, NULL);
    }
}
