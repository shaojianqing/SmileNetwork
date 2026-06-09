#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../logger/logger.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"

#include "activator.h"
#include "bias.h"
#include "vector.h"
#include "matrix.h"
#include "loss.h"
#include "layer.h"

extern Logger logger;

static void releaseBaselayer(BaseLayer *baseLayer);

static bool prepareBaselayer(BaseLayer *baseLayer, LayerConfig config);

static Result* input(InputLayer *this, Vector *vector);

static Result* output(OutputLayer *this);

static Result* loss(OutputLayer *this, Vector *expect);

static Result* backwardInner(BaseLayer *this, Vector *target);

static Result* backwardOutput(BaseLayer *this, Vector *target);

static Result* forwardInner(BaseLayer *this, Vector *vector);

static Result* forwardOutput(BaseLayer *this, Vector *vector);

static Result* optimizeInner(BaseLayer *this, float learnRate);

InputLayer *buildInputLayer(LayerConfig config) {
    InputLayer *inputLayer = (InputLayer*)allocate(sizeof(InputLayer));
    if (inputLayer != NULL) {
        inputLayer->input = input;

        BaseLayer *baseLayer = (BaseLayer*)inputLayer;
        baseLayer->backward = backwardInner;
        baseLayer->optimize = optimizeInner;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseInputLayer(inputLayer);
            return NULL;
        }
    }
    return inputLayer;
}

OutputLayer *buildOutputLayer(LayerConfig config) {
    OutputLayer *outputLayer = (OutputLayer*)allocate(sizeof(OutputLayer));
    if (outputLayer != NULL) {
        outputLayer->output = output;
        outputLayer->loss = loss;

        BaseLayer *baseLayer = (BaseLayer*)outputLayer;
        baseLayer->forward = forwardOutput;
        baseLayer->backward = backwardOutput;
        baseLayer->optimize = optimizeInner;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseOutputLayer(outputLayer);
            return NULL;
        }

        if (config.isOutputLayer) {
            Activator *activator = getActivatorByActivatorLossKind(config.activatorLossKind);
            if (activator != NULL) {
                baseLayer->activator = activator;
            } else {
                logger.error("get activator failure when building output layer^o^");
                releaseOutputLayer(outputLayer);
                return NULL;
            }

            ActivatorLossFunc activatorLossFunc = getActivatorLossFunc(config.activatorLossKind);
            if (activatorLossFunc != NULL) {
                outputLayer->activatorLossFunc = activatorLossFunc;
            } else {
                logger.error("get activator loss func failure when building output layer^o^");
                releaseOutputLayer(outputLayer);
                return NULL;
            }

            ActivatorGradientFunc activatorGradientFunc = getActivatorGradientFunc(config.activatorLossKind);
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

HiddenLayer *buildHiddenLayer(LayerConfig config) {
    HiddenLayer *hiddenLayer = (HiddenLayer*)allocate(sizeof(HiddenLayer));
    if (hiddenLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)hiddenLayer;
        baseLayer->forward = forwardInner;
        baseLayer->backward = backwardInner;
        baseLayer->optimize = optimizeInner;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseHiddenLayer(hiddenLayer);
            return NULL;
        }
    }
    return hiddenLayer;
}

static bool prepareBaselayer(BaseLayer *baseLayer, LayerConfig config) {
    Matrix *modelMatrix = createMatrix(config.matrixRowCount, config.matrixColumnCount, matrixGenerator);
    if (modelMatrix != NULL) {
        baseLayer->modelMatrix = modelMatrix;
    } else {
        logger.error("create model metrix failure when build base layer for memory allocation error^o^");
        return false;
    }

    Bias *modelBias = createBias(config.biasDimensionCount, biasGenerator);
    if (modelBias != NULL) {
        baseLayer->modelBias = modelBias;
    } else {
        logger.error("create model bias failure when build base layer for memory allocation error^o^");
        return false;
    }

    Activator *activator = getActivator(config.activatorKind);
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

void releaseHiddenLayer(HiddenLayer *hiddenLayer) {
    if (hiddenLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)hiddenLayer;
        releaseBaselayer(baseLayer);
        release(hiddenLayer);
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

static Result* input(InputLayer *this, Vector *vector) {
    return forwardInner((BaseLayer*)this, vector);
}

static Result* output(OutputLayer *this) {
    BaseLayer *baseLayer = (BaseLayer*)this;
    Vector *resultVector = baseLayer->resultVector;
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, baseLayer->resultVector);
}

static Result* loss(OutputLayer *this, Vector *expect) {
    if (this->activatorLossFunc == NULL) {
        char *message = "loss func not configured for loss calculation^o^";
        return createResultWithoutData(LOSSFUNC_NO_CONFIG, message);
    }

    BaseLayer *baseLayer = (BaseLayer*)this;
    return this->activatorLossFunc(baseLayer->resultVector, expect);
}

static Result* forwardInner(BaseLayer *this, Vector *vector) {
    Activator *activator = this->activator;
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;

    this->inputVector = vector;
    
    Result *matrixMulResult = matrix->mulVector(matrix, vector);
    if (!matrixMulResult->success(matrixMulResult)) {
        return matrixMulResult;
    }

    Vector *innerVector = (Vector*)matrixMulResult->data;
    releaseResult(matrixMulResult);
    Result *addBiasResult = innerVector->addBias(innerVector, bias);
    if (!addBiasResult->success(addBiasResult)) {
        return addBiasResult;
    }
    releaseResult(addBiasResult);

    Result *activateResult = activator->activate(innerVector);
    releaseVector(innerVector);

    if (!activateResult->success(activateResult)) {
        return activateResult;
    }
    this->resultVector = (Vector*)activateResult->getData(activateResult);
    releaseResult(activateResult);
    
    if (this->nextLayer != NULL) {
        BaseLayer *nextLayer = this->nextLayer;
        return nextLayer->forward(nextLayer, this->resultVector);
    } else {
        return createResultWithoutData(SUCCESS, NULL);
    }
}

static Result* forwardOutput(BaseLayer *this, Vector *vector) {
    Matrix *matrix = this->modelMatrix;
    Bias *bias = this->modelBias;
    
    this->inputVector = vector;
    
    Result *matrixMulResult = matrix->mulVector(matrix, vector);
    if (!matrixMulResult->success(matrixMulResult)) {
        return matrixMulResult;
    }
    Vector *innerVector = (Vector*)matrixMulResult->data;
    releaseResult(matrixMulResult);

    Result *addBiasResult = innerVector->addBias(innerVector, bias);
    if (!addBiasResult->success(addBiasResult)) {
        return addBiasResult;
    }
    releaseResult(addBiasResult);

    Activator *activator = this->activator;
    Result *activateResult = activator->activate(innerVector);
    releaseVector(innerVector);

    if (!activateResult->success(activateResult)) {
        return activateResult;
    }
    this->resultVector = (Vector*)activateResult->getData(activateResult);
    releaseResult(activateResult);

    return createResultWithoutData(SUCCESS, NULL);
}
    
static Result* backwardInner(BaseLayer *this, Vector *prevGradientVector) {
    if (this->activator == NULL) {
        char *message = "activator instance not configured for gradient calculation^o^";
        return createResultWithoutData(GRADFUNC_NO_CONFIG, message);
    }

    Vector *resultVector = this->resultVector;
    Result *derivativeResult = this->activator->derivative(resultVector);
    if (!derivativeResult->success(derivativeResult)) {
        return derivativeResult;
    }

    Vector *thisGradientVector = derivativeResult->getData(derivativeResult);
    releaseResult(derivativeResult);

    Result *mulHamdResult = thisGradientVector->mulHamd(thisGradientVector, prevGradientVector);
    if (!mulHamdResult->success(mulHamdResult)) {
        return mulHamdResult;
    }
    Vector *gradientVector = (Vector *)mulHamdResult->getData(mulHamdResult);
    releaseResult(mulHamdResult);
    
    Vector *inputVector = this->inputVector;
    Result *matrixMulResult = gradientVector->matrixMul(gradientVector, inputVector);
    if (!matrixMulResult->success(matrixMulResult)) {
        return matrixMulResult;
    }
    
    this->gradientMatrix = (Matrix*)matrixMulResult->getData(matrixMulResult);
    releaseResult(matrixMulResult);

    Bias *gradientBias = createBias(gradientVector->count, NULL);
    Result *biasCopyResult = gradientBias->copy(gradientBias, gradientVector);
    if (!biasCopyResult->success(biasCopyResult)) {
        return biasCopyResult;
    }
    
    this->gradientBias = gradientBias;
    releaseResult(biasCopyResult);

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Result *transposeResult = modelMatrix->transpose(modelMatrix);
        if (!transposeResult->success(transposeResult)) {
            return transposeResult;
        }
        Matrix *transposeMatrix = (Matrix*)transposeResult->getData(transposeResult);
        releaseResult(transposeResult);

        Result *mulVectorResult = transposeMatrix->mulVector(transposeMatrix, gradientVector);
        releaseMatrix(transposeMatrix);

        if (!mulVectorResult->success(mulVectorResult)) {
            return mulVectorResult;
        }

        Vector *prevGradientVector = (Vector*)mulVectorResult->getData(mulVectorResult);
        releaseResult(mulVectorResult);

        return prevLayer->backward(prevLayer, prevGradientVector);
    } else {
        return createResultWithoutData(SUCCESS, NULL);
    }
}

static Result* backwardOutput(BaseLayer *this, Vector *target) {
    OutputLayer *outputLayer = (OutputLayer*)this;
    if (outputLayer->activatorGradientFunc == NULL) {
        char *message = "gradient func not configured for gradient calculation^o^";
        return createResultWithoutData(GRADFUNC_NO_CONFIG, message);
    }

    target->printVector(target, "target vector:", 10);

    Vector *resultVector = this->resultVector;
    resultVector->printVector(resultVector, "output layer resultVector:", 10);

    Result *gradientResult = outputLayer->activatorGradientFunc(this->resultVector, target);
    if (!gradientResult->success(gradientResult)) {
        return gradientResult;
    }
    
    Vector *gradientVector = (Vector*)gradientResult->getData(gradientResult);
    releaseResult(gradientResult);

    gradientVector->printVector(gradientVector, "output layer gradientVector:", 10);

    Vector *inputVector = this->inputVector;
    Result *matrixMulResult = gradientVector->matrixMul(gradientVector, inputVector);
    if (!matrixMulResult->success(matrixMulResult)) {
        return matrixMulResult;
    }
    
    this->gradientMatrix = (Matrix*)matrixMulResult->getData(matrixMulResult);
    releaseResult(matrixMulResult);

    Bias *gradientBias = createBias(gradientVector->count, NULL);
    Result *biasCopyResult = gradientBias->copy(gradientBias, gradientVector);
    if (!biasCopyResult->success(biasCopyResult)) {
        return biasCopyResult;
    }

    this->gradientBias = gradientBias;
    releaseResult(biasCopyResult);

    BaseLayer *prevLayer = this->prevLayer;
    if (prevLayer != NULL) {
        Matrix *modelMatrix = this->modelMatrix;
        Result *transposeResult = modelMatrix->transpose(modelMatrix);
        if (!transposeResult->success(transposeResult)) {
            return transposeResult;
        }
        Matrix *transposeMatrix = (Matrix*)transposeResult->getData(transposeResult);
        releaseResult(transposeResult);

        Result *mulVectorResult = transposeMatrix->mulVector(transposeMatrix, gradientVector);
        releaseMatrix(transposeMatrix);

        if (!mulVectorResult->success(mulVectorResult)) {
            return mulVectorResult;
        }
        Vector *prevGradientVector = (Vector*)mulVectorResult->getData(mulVectorResult);
        releaseResult(mulVectorResult);
        
        return prevLayer->backward(prevLayer, prevGradientVector);
    } else {
        return createResultWithoutData(SUCCESS, NULL);
    }
}

static Result* optimizeInner(BaseLayer *this, float learnRate) {
    Matrix *gradientMatrix = this->gradientMatrix;
    Result *result = gradientMatrix->mulNumber(gradientMatrix, learnRate);
    if (!result->success(result)) {
        return result;
    }
    releaseResult(result);

    Matrix *modelMatrix = this->modelMatrix;
    result = modelMatrix->subMatrix(modelMatrix, gradientMatrix);
    if (!result->success(result)) {
        return result;
    }
    releaseResult(result);

    Bias *gradientBias = this->gradientBias;
    result = gradientBias->mulNumber(gradientBias, learnRate);
    if (!result->success(result)) {
        return result;
    }
    releaseResult(result);

    Bias *modelBias = this->modelBias;
    result = modelBias->subBias(modelBias, gradientBias);
    if (!result->success(result)) {
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
