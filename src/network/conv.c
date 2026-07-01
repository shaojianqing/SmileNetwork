#include <float.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
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
#include "linear.h"
#include "tensor.h"
#include "loss.h"
#include "conv.h"

#define CNN_DIMENSION       3

typedef struct TensorLayer TensorLayer;

struct TensorLayer {

    TensorLayer *prevLayer;

    TensorLayer *nextLayer;

    void (*forward)(TensorLayer *this, Tensor *tensor);

    void (*backward)(TensorLayer *this, Tensor *tensor);

    void (*optimize)(TensorLayer *this, float learnRate);
};

struct ConvLayer {

    TensorLayer baseLayer;

    int fieldSize;

    int kernelCount;

    int channelCount;

    Tensor **kernelList;

    Tensor **gradientList;

    Tensor *inputTensor;

    Tensor *resultTensor;
};

struct PoolLayer {

    TensorLayer baseLayer;

    int poolSize;

    int poolStride;
};

struct ConvOutputLayer {

    TensorLayer baseLayer;

    Bias *modelBias;

    Matrix *modelMatrix;

    Bias *gradientBias;

    Matrix *gradientMatrix;

    ActivatorLossFunc activatorLossFunc;

    ActivatorGradientFunc activatorGradientFunc;
};

static Exception MemoryAllocException = {MemoryAllocExceptionType};
static Exception ConfigErrorException = {ConfigErrorExceptionType};

static void forwardConv(TensorLayer *this, Tensor *inputTensor);

static void backwardConv(TensorLayer *this, Tensor *prevGradient);

static void optimizeConv(TensorLayer *this, float learnRate);

static void forwardPool(TensorLayer *this, Tensor *inputTensor);

static void backwardPool(TensorLayer *this, Tensor *inputTensor);

static void optimizePool(TensorLayer *this, float learnRate);

static void forwardOutput(TensorLayer *this, Tensor *inputTensor);

static void backwardOutput(TensorLayer *this, Tensor *inputTensor);

static void optimizeOutput(TensorLayer *this, float learnRate);

static int calculateFeatureMapWidth(Tensor *tensor, int size);

static int calculateFeatureMapHeight(Tensor *tensor, int size);

static float calculateConvolution(ConvLayer *this, Tensor *source, Tensor *kernal, int x, int y);

static float getPoolValue(Tensor *inputTensor, int channel, int x, int y, int poolSize);

static Vector* flattenTensor(Tensor *inputTensor);

ConvLayer* buildConvLayer(ConvLayerConfig *config) {
    ConvLayer *convLayer = (ConvLayer*)allocate(sizeof(ConvLayer));
    if (convLayer == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for convolution layer creation.");
    }

    TensorLayer *baseLayer = (TensorLayer*)convLayer;
    baseLayer->forward = forwardConv;
    baseLayer->backward = backwardConv;
    baseLayer->optimize = optimizeConv;

    convLayer->fieldSize = getConvLayerFieldSize(config);
    convLayer->kernelCount = getConvLayerKernelCount(config);
    convLayer->channelCount = getConvLayerChannelCount(config);
    convLayer->kernelList = (Tensor**)allocate(sizeof(Tensor*)*convLayer->kernelCount);
    if (convLayer->kernelList == NULL) {
        releaseConvLayer(convLayer);
        throw(&MemoryAllocException, "can not allocate memory for kernel list creation in convolution layer.");
    }

    int kernelCount = convLayer->kernelCount;
    int channelCount = convLayer->channelCount;
    int fieldSize = convLayer->fieldSize;
    for (int i=0;i<kernelCount;++i) {
        int kernelShape[CNN_DIMENSION] = {channelCount, fieldSize, fieldSize};
        convLayer->kernelList[i] = createTensor(CNN_DIMENSION, kernelShape, tensorGenerator);
    }

    convLayer->gradientList = (Tensor**)allocate(sizeof(Tensor*)*kernelCount);
    if (convLayer->gradientList == NULL) {
        releaseConvLayer(convLayer);
        throw(&MemoryAllocException, "can not allocate memory for gradient list creation in convolution layer.");
    }

    for (int i=0;i<kernelCount;++i) {
        int shape[CNN_DIMENSION] = {channelCount, fieldSize, fieldSize};
        convLayer->gradientList[i] = createTensor(CNN_DIMENSION, shape, zeroGenerator);
    }

    return convLayer;
}

PoolLayer* buildPoolLayer(ConvLayerConfig *config) {
    PoolLayer *poolLayer = (PoolLayer*)allocate(sizeof(PoolLayer));
    if (poolLayer == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for pool layer creation.");
    }

    TensorLayer *baseLayer = (TensorLayer*)poolLayer;
    baseLayer->forward = forwardPool;
    baseLayer->backward = backwardPool;
    baseLayer->optimize = optimizePool;

    poolLayer->poolSize = getConvLayerPoolSize(config);
    poolLayer->poolStride = getConvLayerPoolStride(config);

    return poolLayer;
}

ConvOutputLayer* buildConvOutputLayer(ConvOutputLayerConfig *config) {
    ConvOutputLayer *outputLayer = (ConvOutputLayer*)allocate(sizeof(ConvOutputLayer));
    if (outputLayer == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for convolution output layer creation.");
    }

    TensorLayer *baseLayer = (TensorLayer*)outputLayer;
    baseLayer->forward = forwardOutput;
    baseLayer->backward = backwardOutput;
    baseLayer->optimize = optimizeOutput;

    int matrixConfigRowCount = getConvOutputMatrixConfigRowCount(config);
    int matrixConfigColumnCount = getConvOutputMatrixConfigColumnCount(config);
    outputLayer->modelMatrix = createMatrix(matrixConfigRowCount, matrixConfigColumnCount, matrixGenerator);

    int biasDimensionCount = getConvOutputBiasConfigDimensionCount(config);
    outputLayer->modelBias = createBias(biasDimensionCount, biasGenerator);

    ActivatorLossKind activatorLossKind = getConvOutputConfigActivatorLossKind(config);
    ActivatorLossFunc activatorLossFunc = getActivatorLossFunc(activatorLossKind);
    if (activatorLossFunc != NULL) {
        outputLayer->activatorLossFunc = activatorLossFunc;
    } else {
        releaseConvOutputLayer(outputLayer);
        throw(&ConfigErrorException, "activator loss func configuration error when building output layer!");
    }

    ActivatorGradientFunc activatorGradientFunc = getActivatorGradientFunc(activatorLossKind);
    if (activatorGradientFunc != NULL) {
        outputLayer->activatorGradientFunc = activatorGradientFunc;
    } else {
        releaseConvOutputLayer(outputLayer);
        throw(&ConfigErrorException, "activator gradient func configuration error when building output layer!");
    }

    return outputLayer;
}

void releaseConvLayer(ConvLayer *convLayer) {
    if (convLayer != NULL) {
        release(convLayer->kernelList);
        release(convLayer->gradientList);
        release(convLayer);
    }
}

void releasePoolLayer(PoolLayer *poolLayer) {
    if (poolLayer != NULL) {
        release(poolLayer);
    }
}

void releaseConvOutputLayer(ConvOutputLayer *outputLayer) {
    if (outputLayer != NULL) {
        release(outputLayer->modelMatrix);
        release(outputLayer->modelBias);
        release(outputLayer);
    }
}

void inputConv(ConvLayer *this, Tensor *tensor) {
    TensorLayer *tensorLayer = (TensorLayer*)this;
    forwardConv(tensorLayer, tensor);
}

void setNextTensorLayer(TensorLayer *this, TensorLayer *nextLayer) {
    assertNotNull(this, "this tensor layer is null for setting next tensor layer!");
    assertNotNull(nextLayer, "next tensor layer is null for setting next tensor layer!");

    this->nextLayer = nextLayer;
}

void setPrevTensorLayer(TensorLayer *this, TensorLayer *prevLayer) {
    assertNotNull(this, "this tensor layer is null for setting next tensor layer!");
    assertNotNull(prevLayer, "prev tensor layer is null for setting prev tensor layer!");

    this->prevLayer = prevLayer;
}

static void forwardConv(TensorLayer *this, Tensor *inputTensor) {

    assertNotNull(this, "this tensor layer is null for convolution forward!");
    assertNotNull(inputTensor, "this input tensor instance is null for convolution forward!");

    int *inputTensorShape = getTensorShape(inputTensor);
    int inputChannelCount = inputTensorShape[0];

    ConvLayer *convLayer = (ConvLayer*)this;

    assertDataMatch(inputChannelCount, convLayer->channelCount, "channel count does not match for input tensor!");

    convLayer->inputTensor = inputTensor;
    int kernelCount = convLayer->kernelCount;
    Tensor **kernelList = convLayer->kernelList;

    int mapWidth = calculateFeatureMapWidth(inputTensor, convLayer->fieldSize);
    int mapHeight = calculateFeatureMapHeight(inputTensor, convLayer->fieldSize);

    int resultMapIndex[CNN_DIMENSION] = {0,0,0};
    int resultMapShape[CNN_DIMENSION] = {kernelCount, mapWidth, mapHeight};
    Tensor *resultMap = createTensor(CNN_DIMENSION, resultMapShape, NULL);
    
    int x = 0, y = 0;
    for (int i=0;i<kernelCount;++i) {
        Tensor *kernel = kernelList[i];
        for (int j=0;j<mapHeight;++j) {
            for (int k=0;k<mapWidth;++k) {
                resultMapIndex[0] = i;
                resultMapIndex[1] = j;
                resultMapIndex[2] = k;

                float value = calculateConvolution(convLayer, inputTensor, kernel, x, y);
                setTensorValue(resultMap, resultMapIndex, value);
                x++;
            }
            y++;
        }
    }

    TensorLayer *nextLayer = (TensorLayer*)this->nextLayer;
    if (nextLayer != NULL) {
        nextLayer->forward(nextLayer, resultMap);
        releaseTensor(resultMap);
    }
}

static void backwardConv(TensorLayer *this, Tensor *nextGradient) {

    assertNotNull(this, "this tensor layer is null for convolution backward!");

    ConvLayer *convLayer = (ConvLayer*)this;

    int kernelCount = convLayer->kernelCount;
    Tensor *inputTensor = convLayer->inputTensor;
    int mapWidth = calculateFeatureMapWidth(inputTensor, convLayer->fieldSize);
    int mapHeight = calculateFeatureMapHeight(inputTensor, convLayer->fieldSize);

    int *nextGradientShape = getTensorShape(nextGradient);
    int gradientChannelCount = nextGradientShape[0];
    assertDataMatch(gradientChannelCount, kernelCount, "channel count does not match for gradient tensor!");

    int gradientWidth = nextGradientShape[1];
    int gradientHeight = nextGradientShape[2];

    assertDataMatch(mapWidth, gradientWidth, "gradient width and map width does not match for gradient tensor!");
    assertDataMatch(mapHeight, gradientHeight, "gradient height and map height does not match for gradient tensor!");

    int x = 0, y = 0;
    int fieldSize = convLayer->fieldSize;
    for (int i=0;i<kernelCount;++i) {
        for (int j=0;j<gradientHeight;++j) {
            for (int k=0;k<gradientWidth;++k) {

                int nextGradidentIndex[CNN_DIMENSION] = {i, x, y};
                float nextGradientValue = getTensorValue(nextGradient, nextGradidentIndex);

                int inputIndex[CNN_DIMENSION] = {0, x, y};
                float inputValue = getTensorValue(inputTensor, inputIndex);

                float thisGradientValue = inputValue * nextGradientValue;

                Tensor *thisGradient = convLayer->gradientList[i];
                
                int thisGradientIndex[CNN_DIMENSION] = {i, x%fieldSize, y%fieldSize};
                float totalGradientValue = getTensorValue(thisGradient, thisGradientIndex);
                totalGradientValue += thisGradientValue;
                setTensorValue(thisGradient, thisGradientIndex, totalGradientValue);

                x++;
            }
            y++;
        }
    }

    TensorLayer *prevLayer = (TensorLayer*)this->prevLayer;
    if (prevLayer != NULL) {
        Tensor **kernelList = convLayer->kernelList;
        int *inputTensorShape = getTensorShape(inputTensor);
        Tensor *prevGradient = createTensor(CNN_DIMENSION, inputTensorShape, zeroGenerator);

        int *nextTensorShape = getTensorShape(nextGradient);
        int nextGradientWidth = nextTensorShape[1];
        int nextGradientHeight = nextTensorShape[2];

        int x = 0, y = 0;
        int kernelIndex[CNN_DIMENSION] = {0, 0, 0};
        int nextGradientIndex[CNN_DIMENSION] = {0, 0, 0};
        int prevGradientIndex[CNN_DIMENSION] = {0, 0, 0};
        for (int i=0;i<kernelCount;++i) {
            for (int j=0;j<nextGradientHeight;++j) {
                for (int k=0;k<nextGradientWidth;++k) {
                
                    Tensor *kernel = kernelList[i];
                    for (int m=0;m<fieldSize;++m) {
                        for (int n=0;n<fieldSize;++n) {
                            kernelIndex[0] = i;
                            kernelIndex[1] = m;
                            kernelIndex[2] = n;

                            float kernelValue = getTensorValue(kernel, kernelIndex);

                            nextGradientIndex[0] = i;
                            nextGradientIndex[1] = j;
                            nextGradientIndex[2] = k;
                            float nextGradientValue = getTensorValue(nextGradient, nextGradientIndex);

                            prevGradientIndex[0] = i;
                            prevGradientIndex[1] = j + m;
                            prevGradientIndex[2] = k + n;

                            float prevGradientValue = getTensorValue(prevGradient, prevGradientIndex);
                            prevGradientValue += kernelValue * nextGradientValue;

                            setTensorValue(prevGradient, prevGradientIndex, prevGradientValue);
                        }
                    }
                }
            }
        }
        prevLayer->backward(prevLayer, prevGradient);
    }
}

static void optimizeConv(TensorLayer *this, float learnRate) {

    assertNotNull(this, "this tensor layer is null for convolution optimization!");

    ConvLayer *convLayer = (ConvLayer*)this;
    for (int i=0;i<convLayer->kernelCount;++i) {
        Tensor *kernal = convLayer->kernelList[i];
        Tensor *gradident = convLayer->gradientList[i];

        mulTensorNumber(gradident, learnRate);
        subTensor(kernal, gradident);
    }
}

static void forwardPool(TensorLayer *this, Tensor *inputTensor) {

    PoolLayer *poolLayer = (PoolLayer*)this;
    int *inputShape = getTensorShape(inputTensor);
    int channelCount = inputShape[0];
    int inputMapWidth = inputShape[1];
    int inputMapHeight = inputShape[2];

    int poolSize = poolLayer->poolSize;
    int poolMapWidth = inputMapWidth/poolSize;
    int poolMapHeight = inputMapHeight/poolSize;

    int poolMapIndex[CNN_DIMENSION] = {0, 0, 0};
    int poolMapShape[CNN_DIMENSION] = {channelCount, poolMapWidth, poolMapHeight};
    Tensor *poolMapTensor = createTensor(CNN_DIMENSION, poolMapShape, zeroGenerator);
    for (int i=0;i<channelCount;++i) {
        for (int x=0;x<poolMapHeight;++x) {
            for (int y=0;y<poolMapWidth;++y) {
                poolMapIndex[0] = i;
                poolMapIndex[1] = y;
                poolMapIndex[2] = x;
                float poolValue = getPoolValue(inputTensor, i, x, y, poolSize);
                setTensorValue(poolMapTensor, poolMapIndex, poolValue);
            }
        }
    }

    TensorLayer *nextLayer = (TensorLayer*)this->nextLayer;
    if (nextLayer != NULL) {
        nextLayer->forward(nextLayer, poolMapTensor);
    }
    releaseTensor(poolMapTensor);
}

static void backwardPool(TensorLayer *this, Tensor *inputTensor) {

}

static void optimizePool(TensorLayer *this, float learnRate) {

}

static void forwardOutput(TensorLayer *this, Tensor *inputTensor) {

}

static void backwardOutput(TensorLayer *this, Tensor *inputTensor) {

}

static void optimizeOutput(TensorLayer *this, float learnRate) {

}

static float calculateConvolution(ConvLayer *this, Tensor *source, Tensor *kernal, int x, int y) {

    int dimension = getDimension(source);
    assertDataMatch(dimension, CNN_DIMENSION, "tensor dimension is not equal to the expected setting value!");

    int fieldSize = this->fieldSize;

    int *sourceShape = getTensorShape(source);
    int channelCount = sourceShape[0];
    int sourceIndex[CNN_DIMENSION] = {0, 0, 0};
    int kernelIndex[CNN_DIMENSION] = {0, 0, 0};

    float result = 0.0;
    for (int i=0;i<channelCount;++i) {
        for (int j=0;j<fieldSize;++j) {
            for (int k=0;k<fieldSize;++k) {
                sourceIndex[0] = i;
                sourceIndex[1] = x + j;
                sourceIndex[2] = y + k;

                kernelIndex[0] = i;
                kernelIndex[1] = j;
                kernelIndex[2] = k;

                float sourceValue = getTensorValue(source, sourceIndex);
                float kernelValue = getTensorValue(kernal, kernelIndex);

                result += sourceValue * kernelValue;
            }
        }
    }

    return result;
}

static int calculateFeatureMapWidth(Tensor *tensor, int size) {

    assertNotNull(tensor, "this tensor layer is null for calculating feature map width!");

    int dimension = getDimension(tensor);
    assertDataMatch(dimension, CNN_DIMENSION, "tensor dimension is not equal to the expected setting value!");

    int *shape = getTensorShape(tensor);
    int width = shape[1];

    return (width - size + 1);
}

static int calculateFeatureMapHeight(Tensor *tensor, int size) {

    assertNotNull(tensor, "this tensor layer is null for calculating feature map height!");

    int dimension = getDimension(tensor);
    assertDataMatch(dimension, CNN_DIMENSION, "tensor dimension is not equal to the expected setting value!");

    int *shape = getTensorShape(tensor);
    int height = shape[2];

    return (height - size + 1);
}

static float getPoolValue(Tensor *inputTensor, int channel, int x, int y, int poolSize) {
    float result = FLT_MIN;
    int inputMapIndex[CNN_DIMENSION] = {0, 0, 0};
    for (int i=0;i<poolSize;++i) {
        for (int j=0;j<poolSize;++j) {
            inputMapIndex[0] = channel;
            inputMapIndex[2] = x*poolSize + i;
            inputMapIndex[1] = y*poolSize + j;
            float value = getTensorValue(inputTensor, inputMapIndex);
            if (result<value) {
                result = value;
            }
        }
    }
    return result;
}

static Vector* flattenTensor(Tensor *inputTensor) {
    return NULL;
}