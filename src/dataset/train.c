#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"
#include "../network/bias.h"
#include "../network/vector.h"

#include "mnist.h"
#include "train.h"

#define BYTE_FLOAT_FACTOR       256.0

static Vector* selectAndGenerateData(MnistData *mnistData, int index);

static Vector* selectAndGenerateLabel(MnistLabel *mnistLabel, int index);

static void setTrainData(TrainBatch* trainBatch, int index, Vector *data, Vector *label);

Result* loadTrainBatchStochastic(int batchSize) {
    MnistData *mnistData = getMnistTrainData();
    MnistLabel *mnistLabel = getMnistTrainLabel();

    if (mnistData == NULL) {
        char *message = "Mnist data is not loaded ready to load train batch^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (mnistLabel == NULL) {
        char *message = "Mnist label is not loaded ready to load train batch^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (mnistData->imageCount != mnistLabel->labelCount) {
        char *message = "Mnist data and label count does not match, can not load train batch^o^";
        return createResultWithoutData(MNIST_NOT_MATCH, message);
    }

    TrainBatch *trainBatch = (TrainBatch*)allocate(sizeof(TrainBatch));
    if (trainBatch == NULL) {
        char *message = "Can not create train batch instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    trainBatch->dataCount = batchSize;
    trainBatch->dataList = (TrainData*)allocate(batchSize*sizeof(TrainData));
    if (trainBatch->dataList == NULL) {
        char *message = "Can not allocate train batch datalist for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<batchSize;++i) {
        Vector *data = selectAndGenerateData(mnistData, i);
        Vector *label = selectAndGenerateLabel(mnistLabel, i);

        setTrainData(trainBatch, i, data, label);
    }
    return createResultWithData(SUCCESS, NULL, TYPE_TRAIN_BATCH, trainBatch);
}

void releaseTrainBatch(TrainBatch *trainBatch) {
    if (trainBatch != NULL) {
        for (int i=0;i<trainBatch->dataCount;++i) {
            TrainData trainData = trainBatch->dataList[i];
            releaseVector(trainData.data);
            releaseVector(trainData.label);
        }
        release(trainBatch->dataList);
        release(trainBatch);
    }
}

static Vector* selectAndGenerateData(MnistData *mnistData, int index) {
    int dimensionCount = mnistData->rowCount*mnistData->columnCount;
    Vector *data = createVector(dimensionCount);
    if (data != NULL) {
        for (int i=0;i<dimensionCount;++i) {
            byte *buffer = mnistData->dataBuffer;
            byte valueByte = buffer[index * dimensionCount + i];
            float valueFloat = valueByte/BYTE_FLOAT_FACTOR;
            data->setValue(data, i, valueFloat);
        }
    }
    return data;
}

static Vector* selectAndGenerateLabel(MnistLabel *mnistLabel, int index) {
    int dimensionCount = 10;
     Vector *label = createVector(dimensionCount);
     if (label != NULL) {
        for (int i=0;i<dimensionCount;++i) {
            int valueByte = mnistLabel->labelBuffer[index];
            if (i == valueByte) {
                label->setValue(label, i, 1.0);
            } else {
                label->setValue(label, i, 0.0);
            }
        }
     }
     return label;
}

static void setTrainData(TrainBatch* trainBatch, int index, Vector *data, Vector *label) {
    if (trainBatch == NULL || data == NULL || label == NULL) {
        return;
    }

    if (index < 0 || index >= trainBatch->dataCount) {
        return;
    }

    trainBatch->dataList[index].data = data;
    trainBatch->dataList[index].label = label;
}