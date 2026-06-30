#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"
#include "../network/bias.h"
#include "../network/vector.h"

#include "mnist.h"
#include "train.h"

#define BYTE_FLOAT_FACTOR       256.0

struct TrainData {

    Vector *data;

    Vector *label;
};

struct TrainBatch {

    int dataCount;

    TrainData *dataList;
};

static Exception MemoryAllocException = {MemoryAllocExceptionType};

static Vector* selectAndGenerateData(MnistData *mnistData, int index);

static Vector* selectAndGenerateLabel(MnistLabel *mnistLabel, int index);

static void setTrainData(TrainBatch* trainBatch, int index, Vector *data, Vector *label);

TrainBatch* loadTrainBatchStochastic(int batchSize) {
    MnistData *mnistData = getMnistTrainData();
    MnistLabel *mnistLabel = getMnistTrainLabel();

    assertNotNull(mnistData, "Mnist data is not loaded ready to load train batch!");
    assertNotNull(mnistLabel, "Mnist label is not loaded ready to load train batch!");

    assertDataMatch(getImageCount(mnistData), getLableCount(mnistLabel), "vMnist data and label count does not match, can not load train batch!");

    TrainBatch *trainBatch = (TrainBatch*)allocate(sizeof(TrainBatch));
    if (trainBatch == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for train batch creation.");
    }

    trainBatch->dataCount = batchSize;
    trainBatch->dataList = (TrainData*)allocate(batchSize*sizeof(TrainData));
    if (trainBatch->dataList == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for train batch datalist creation.");
    }

    int imageCount = getImageCount(mnistData);
    for (int i=0;i<batchSize;++i) {
        int index = rand()%imageCount;
        Vector *data = selectAndGenerateData(mnistData, index);
        Vector *label = selectAndGenerateLabel(mnistLabel, index);

        setTrainData(trainBatch, i, data, label);
    }
    return trainBatch;
}

TrainBatch* loadTrainBatchForValidate() {
    MnistData *mnistData = getMnistTrainData();
    MnistLabel *mnistLabel = getMnistTrainLabel();

    assertNotNull(mnistData, "Mnist data is not loaded ready to load validation batch!");
    assertNotNull(mnistLabel, "Mnist label is not loaded ready to load validation batch!");

    assertDataMatch(getImageCount(mnistData), 10000, "Mnist label is loaded but not for test or validation!");
    assertDataMatch(getImageCount(mnistData), getLableCount(mnistLabel), "Mnist data and label count does not match, can not load validation batch!");

    TrainBatch *trainBatch = (TrainBatch*)allocate(sizeof(TrainBatch));
    if (trainBatch == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for train batch creation.");
    }

    int totalDataCount = getImageCount(mnistData);
    trainBatch->dataCount = totalDataCount;
    trainBatch->dataList = (TrainData*)allocate(totalDataCount*sizeof(TrainData));
    if (trainBatch->dataList == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for train batch datalist creation.");
    }

    for (int i=0;i<totalDataCount;++i) {
        Vector *data = selectAndGenerateData(mnistData, i);
        Vector *label = selectAndGenerateLabel(mnistLabel, i);

        setTrainData(trainBatch, i, data, label);
    }
    return trainBatch;
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

int getTrainDataCount(TrainBatch *this) {
    if (this != NULL) {
        return this->dataCount;
    }
    return 0;
}

TrainData* getTrainData(TrainBatch *this, int index) {
    if (this != NULL && index < this->dataCount) {
        return &this->dataList[index];
    }
    return NULL;
}

Vector* getDataForTrain(TrainData* this) {
    if (this != NULL) {
        return this->data;
    }
    return NULL;
}

Vector* getLabelForTrain(TrainData* this) {
    if (this != NULL) {
        return this->label;
    }
    return NULL;
}

static Vector* selectAndGenerateData(MnistData *mnistData, int index) {
    int rowCount = getMnistRowCount(mnistData);
    int columnCount = getMnistRowCount(mnistData);
    int dimensionCount = rowCount * columnCount;
    Vector *data = createVector(dimensionCount);
    if (data != NULL) {
        for (int i=0;i<dimensionCount;++i) {
            byte *buffer = getDataBuffer(mnistData);
            byte valueByte = buffer[index * dimensionCount + i];
            float valueFloat = valueByte/BYTE_FLOAT_FACTOR;
            setVectorValue(data, i, valueFloat);
        }
    }
    return data;
}

static Vector* selectAndGenerateLabel(MnistLabel *mnistLabel, int index) {
    int dimensionCount = 10;
     Vector *label = createVector(dimensionCount);
     if (label != NULL) {
        for (int i=0;i<dimensionCount;++i) {
            int valueByte = getLabel(mnistLabel, index);
            if (i == valueByte) {
                setVectorValue(label, i, 1.0);
            } else {
                setVectorValue(label, i, 0.0);
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