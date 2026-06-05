#include <memory.h>
#include <fcntl.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../result/result.h"
#include "../logger/logger.h"
#include "../file/file.h"
#include "../reader/bytereader.h"

#include "mnist.h"

#define MNIST_DATA_MAGIC            2051
#define MNIST_LABEL_MAGIC           2049

extern Logger logger;

static MnistData *mnistTrainData;

static MnistLabel *mnistTrainLabel;

bool loadMnistDataFromFile(const char *filename) {
    File *mnistDataFile = openFile(filename, O_RDONLY);
    if (mnistDataFile == NULL) {
        logger.error("open mnist data file error for existence or permission reason^o^");
        return false;
    }
    
    Result *readResult = mnistDataFile->readByteBuffer(mnistDataFile);
    if (!readResult->success(readResult)) {
        logger.error("read mnist data error, reason:%s", readResult->message);
        closeFile(mnistDataFile);
        releaseResult(readResult);
        return false;
    }

    long bufferSize = mnistDataFile->getFileSize(mnistDataFile);
    byte *dataBuffer = (byte*)readResult->getData(readResult);
    releaseResult(readResult);
    closeFile(mnistDataFile);

    ByteReader *byteReader = createByteReader(dataBuffer, bufferSize, 0);
    if (byteReader == NULL) {
        release(dataBuffer);
        logger.error("create byte reader instance failure for memory allocation error^o^");
        return false;
    }

    int magic = byteReader->readInteger(byteReader);
    int imageCount = byteReader->readInteger(byteReader);
    int rowCount = byteReader->readInteger(byteReader);
    int columnCount = byteReader->readInteger(byteReader);

    if (magic != MNIST_DATA_MAGIC) {
        release(dataBuffer);
        release(byteReader);

        logger.error("mnist data file magic does not match, not legal mnist data file^o^");
        return false;
    }

    mnistTrainData = (MnistData*)allocate(sizeof(MnistData));
    if (mnistTrainData == NULL) {
        release(dataBuffer);
        release(byteReader);
        
        logger.error("create mnist data instance failure for memory allocation error^o^");
        return false;
    }

    mnistTrainData->magic = magic;
    mnistTrainData->imageCount = imageCount;
    mnistTrainData->rowCount = rowCount;
    mnistTrainData->columnCount = columnCount;

    long totalCount = imageCount*rowCount*columnCount;

    mnistTrainData->dataBuffer = (byte*)allocate(totalCount);
    if (mnistTrainData->dataBuffer == NULL) {
        release(mnistTrainData);
        release(dataBuffer);
        release(byteReader);

        logger.error("create mnist data buffer failure for memory allocation error^o^");
        return false;
    }

    memcpy(mnistTrainData->dataBuffer, (dataBuffer + 16), totalCount);
    release(dataBuffer);
    release(byteReader);

    return true;
}

bool loadMnistLabelFromFile(const char *filename) {
    File *mnistLabelFile = openFile(filename, O_RDONLY);
    if (mnistLabelFile == NULL) {
        logger.error("open mnist label file error for existence or permission reason^o^");
        return false;
    }
    
    Result *readResult = mnistLabelFile->readByteBuffer(mnistLabelFile);
    if (!readResult->success(readResult)) {
        logger.error("read mnist label error, reason:%s", readResult->message);
        closeFile(mnistLabelFile);
        releaseResult(readResult);
        return false;
    }

    long bufferSize = mnistLabelFile->getFileSize(mnistLabelFile);
    byte *dataBuffer = (byte*)readResult->getData(readResult);
    releaseResult(readResult);
    closeFile(mnistLabelFile);

    ByteReader *byteReader = createByteReader(dataBuffer, bufferSize, 0);
    if (byteReader == NULL) {
        logger.error("create byte reader instance failure for memory allocation error^o^");
        release(dataBuffer);
        return false;
    }

    int magic = byteReader->readInteger(byteReader);
    int labelCount = byteReader->readInteger(byteReader);

    if (magic != MNIST_LABEL_MAGIC) {
        release(dataBuffer);
        release(byteReader);

        logger.error("mnist label file magic does not match, not legal mnist label file^o^");
        return false;
    }

    mnistTrainLabel = (MnistLabel*)allocate(sizeof(MnistLabel));
    if (mnistTrainLabel == NULL) {
        release(dataBuffer);
        release(byteReader);
        
        logger.error("create mnist label instance failure for memory allocation error^o^");
        return false;
    }

    mnistTrainLabel->labelCount = labelCount;
    mnistTrainLabel->labelBuffer = (byte*)allocate(labelCount);
    if (mnistTrainLabel->labelBuffer == NULL) {
        release(mnistTrainLabel);
        release(dataBuffer);
        release(byteReader);

        logger.error("create mnist label buffer failure for memory allocation error^o^");
        return false;
    }

    memcpy(mnistTrainLabel->labelBuffer, (dataBuffer + 8), labelCount);
    release(dataBuffer);
    release(byteReader);

    return true;
}

MnistData* getMnistTrainData() {
    return mnistTrainData;
}

MnistLabel* getMnistTrainLabel() {
    return mnistTrainLabel;
}