#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../logger/logger.h"
#include "../datatype/stringtype.h"

#include "result.h"
#include "bias.h"
#include "vector.h"
#include "matrix.h"
#include "activator.h"
#include "layer.h"

extern Logger logger;

static void releaseBaselayer(BaseLayer *baseLayer);

static bool prepareBaselayer(BaseLayer *baseLayer, LayerConfig *config);

static Result* input(InputLayer *this, Vector *vector);
    
static Result* backwardInputLayer(InputLayer *this, Vector *vector);

static Result* output(OutputLayer *this);

static Result* forewardOutputLayer(OutputLayer *this, Vector *vector);

static Result* forewardHiddenLayer(BaseLayer *this, Vector *vector);

static Result* backwardHiddenLayer(BaseLayer *this, Vector *vector);

InputLayer *buildInputLayer(LayerConfig *config) {
    InputLayer *inputLayer = (InputLayer*)malloc(sizeof(InputLayer));
    if (inputLayer != NULL) {
        inputLayer->input = input;
        inputLayer->backward = backwardInputLayer;

        BaseLayer *baseLayer = (BaseLayer*)inputLayer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseInputLayer(inputLayer);
            return NULL;
        }
    }
    return inputLayer;
}

OutputLayer *buildOutputLayer(LayerConfig *config) {
    OutputLayer *outputLayer = (OutputLayer*)malloc(sizeof(OutputLayer));
    if (outputLayer != NULL) {
        outputLayer->output = output;
        outputLayer->foreward = forewardOutputLayer;

        BaseLayer *baseLayer = (BaseLayer*)outputLayer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseOutputLayer(outputLayer);
            return NULL;
        }
    }
    return outputLayer;
}

HiddenLayer *buildHiddenLayer(LayerConfig *config) {
    HiddenLayer *hiddenLayer = (HiddenLayer*)malloc(sizeof(HiddenLayer));
    if (hiddenLayer != NULL) {
        hiddenLayer->foreward = forewardHiddenLayer;
        hiddenLayer->backward = backwardHiddenLayer;
        
        BaseLayer *baseLayer = (BaseLayer*)hiddenLayer;
        bool success = prepareBaselayer(baseLayer, config);
        if (!success) {
            releaseHiddenLayer(hiddenLayer);
            return NULL;
        }
    }
    return hiddenLayer;
}

static bool prepareBaselayer(BaseLayer *baseLayer, LayerConfig *config) {
    Matrix *matrix = createMatrix(config->matrixRowCount, config->matrixColumnCount);
    if (matrix != NULL) {
        baseLayer->matrix = matrix;
    } else {
        logger.error("create metrix failure when build input layer for memory allocation error^o^");
        return false;
    }

    Bias *bias = createBias(config->biasDimensionCount);
    if (bias != NULL) {
        baseLayer->bias = bias;
    } else {
        logger.error("create bias failure when build input layer for memory allocation error^o^");
        return false;
    }

    Activator *activator = getActivator(config->activatorKind);
    if (activator != NULL) {
        baseLayer->activator = activator;
    } else {
        logger.error("get activator failure when build input layer^o^");
        return false;
    }

    return true;
}

void releaseInputLayer(InputLayer *inputLayer) {
    if (inputLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)inputLayer;
        releaseBaselayer(baseLayer);
        free(inputLayer);
    }
}

void releaseOutputLayer(OutputLayer *outputLayer) {
    if (outputLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)outputLayer;
        releaseBaselayer(baseLayer);
        free(outputLayer);
    }
}

void releaseHiddenLayer(HiddenLayer *hiddenLayer) {
    if (hiddenLayer != NULL) {
        BaseLayer *baseLayer = (BaseLayer*)hiddenLayer;
        releaseBaselayer(baseLayer);
        free(hiddenLayer);
    }
}

static void releaseBaselayer(BaseLayer *baseLayer) {
    Matrix *matrix = baseLayer->matrix;
    Bias *bias = baseLayer->bias;

    releaseMatrix(matrix);
    releaseBias(bias);
}

static Result* input(InputLayer *this, Vector *vector) {
    return NULL;
}
    
static Result* backwardInputLayer(InputLayer *this, Vector *vector) {
    return NULL;
}

static Result* output(OutputLayer *this) {
    return NULL;
}

static Result* forewardOutputLayer(OutputLayer *this, Vector *vector) {
    return NULL;
}

static Result* forewardHiddenLayer(BaseLayer *this, Vector *vector) {
    return NULL;
}

static Result* backwardHiddenLayer(BaseLayer *this, Vector *vector) {
    return NULL;
}
