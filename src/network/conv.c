#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"

#include "bias.h"
#include "config.h"
#include "vector.h"
#include "linear.h"
#include "tensor.h"
#include "conv.h"

#define CNN_DIMENSION       3

typedef struct TensorLayer TensorLayer;

struct TensorLayer {

    TensorLayer *prevLayer;

    TensorLayer *nextLayer;

    VectorLayer *outputLayer;

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

static Exception MemoryAllocException = {MemoryAllocExceptionType};

static void forwardTensor(TensorLayer *this, Tensor *inputTensor);

static void backwardTensor(TensorLayer *this, Tensor *prevGradient);

static void optimizeTensor(TensorLayer *this, float learnRate);

static void forwardPool(TensorLayer *this, Tensor *inputTensor);

static void backwardPool(TensorLayer *this, Tensor *inputTensor);

static void optimizePool(TensorLayer *this, float learnRate);

static int calculateFeatureMapWidth(Tensor *tensor, int size);

static int calculateFeatureMapHeight(Tensor *tensor, int size);

static Tensor* captureField(Tensor *source, int startIndex[], int size);

static float calculateConvolution(ConvLayer *this, Tensor *source, Tensor *kernal, int x, int y);

ConvLayer* createConvLayer(int fieldSize, int kernelCount, int channelCount) {
    ConvLayer *convLayer = (ConvLayer*)allocate(sizeof(ConvLayer));
    if (convLayer == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for convolution layer creation.");
    }

    TensorLayer *baseLayer = (TensorLayer*)convLayer;
    baseLayer->forward = forwardTensor;
    baseLayer->backward = backwardTensor;
    baseLayer->optimize = optimizeTensor;

    convLayer->kernelCount = kernelCount;
    convLayer->channelCount = channelCount;
    convLayer->kernelList = (Tensor**)allocate(sizeof(Tensor*)*kernelCount);
    if (convLayer->kernelList == NULL) {
        releaseConvLayer(convLayer);
        throw(&MemoryAllocException, "can not allocate memory for kernel list creation in convolution layer.");
    }

    for (int i=0;i<kernelCount;++i) {
        int shape[CNN_DIMENSION] = {channelCount, fieldSize, fieldSize};
        convLayer->kernelList[i] = createTensor(CNN_DIMENSION, shape, tensorGenerator);
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

PoolLayer* createPoolLayer(int poolSize, int poolStride) {
    PoolLayer *poolLayer = (PoolLayer*)allocate(sizeof(PoolLayer));
    if (poolLayer == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for pool layer creation.");
    }

    TensorLayer *baseLayer = (TensorLayer*)poolLayer;
    baseLayer->forward = forwardPool;
    baseLayer->backward = backwardPool;
    baseLayer->optimize = optimizePool;

    poolLayer->poolSize = poolSize;
    poolLayer->poolStride = poolStride;

    return poolLayer;
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

void inputConv(ConvLayer *this, Tensor *tensor) {
    TensorLayer *tensorLayer = (TensorLayer*)this;
    forwardTensor(tensorLayer, tensor);
}

static void forwardTensor(TensorLayer *this, Tensor *inputTensor) {

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
                resultMapIndex[1] = x;
                resultMapIndex[2] = y;

                float value = calculateConvolution(convLayer, inputTensor, kernel, x, y);
                setTensorValue(resultMap, resultMapIndex, value);
                x++;
            }
            y++;
        }
    }

    TensorLayer *nextLayer = (TensorLayer*)this->nextLayer;
    VectorLayer *outputLayer = (VectorLayer*)this->outputLayer;
    if (nextLayer != NULL) {
        nextLayer->forward(nextLayer, resultMap);
        releaseTensor(resultMap);
    } else if (outputLayer != NULL) {
        Vector *result = flattenTenosr(resultMap);
        forward(outputLayer, result);
    }
}

static void backwardTensor(TensorLayer *this, Tensor *nextGradient) {

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

static void optimizeTensor(TensorLayer *this, float learnRate) {

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

    int startIndex[CNN_DIMENSION] = {0, 0, 0};

    


}

static void backwardPool(TensorLayer *this, Tensor *inputTensor) {

}

static void optimizePool(TensorLayer *this, float learnRate) {

}

static Tensor* captureField(Tensor *source, int startIndex[], int size) {
    int dimension = getDimension(source);
    assertDataMatch(dimension, CNN_DIMENSION, "tensor dimension is not equal to the expected setting value!");
    
    int *sourceShape = getTensorShape(source);
    int *fieldShape = (int*)allocate(sizeof(int*)*dimension);

    int channelCount = sourceShape[0];
    fieldShape[0] = channelCount;
    fieldShape[1] = size;
    fieldShape[2] = size;

    int x = startIndex[1];
    int y = startIndex[2];

    int sourceIndex[3], fieldIndex[3];
    Tensor *fieldTensor = createTensor(dimension, fieldShape, NULL);
    for (int i=0;i<channelCount;++i) {
        for (int j=0;j<size;++j) {
            for (int k=0;k<size;++k) {
                sourceIndex[0] = i;
                sourceIndex[1] = x + j;
                sourceIndex[2] = y + k;

                fieldIndex[0] = i;
                fieldIndex[0] = j;
                fieldIndex[0] = k;

                float data = getTensorValue(source, sourceIndex);
                setTensorValue(fieldTensor, fieldIndex, data);
            }
        }
    }

    return fieldTensor;
}

static float calculateConvolution(ConvLayer *this, Tensor *source, Tensor *kernal, int x, int y) {
    int index[CNN_DIMENSION] = {0, x, y};
    Tensor* field = captureField(source, index, this->fieldSize);
    float value = mulTensorScalar(kernal, field);
    releaseTensor(field);

    return value;
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