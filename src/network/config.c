#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../random/random.h"
#include "../memory/memory.h"
#include "../result/result.h"
#include "../dataset/train.h"
#include "../file/file.h"
#include "../json/json.h"

#include "activator.h"
#include "bias.h"
#include "matrix.h"
#include "vector.h"
#include "loss.h"
#include "layer.h"
#include "network.h"
#include "config.h"

static Result* parseConfig(char *content);

Result *loadNetworkConfig(char *filepath) {
    File *configFile = openFile(filepath, O_RDONLY);
    if (configFile == NULL) {
        char *message = "config open network configuration file error for existence or permission reason^o^";
        createResultWithoutData(FILE_OPEN_ERROR, message);
    }

    Result *readResult = configFile->readCharString(configFile);
    if (!readResult->success(readResult)) {
        closeFile(configFile);
        return readResult;
    }
    char *content = (char*)readResult->getData(readResult);
    releaseResult(readResult);
    closeFile(configFile);

    Result *parseResult = parseConfig(content);
    release(content);
    
    return parseResult;
}

static Result* parseConfig(char *content) {
    NetworkConfig *networkConfig = (NetworkConfig *)allocate(sizeof(NetworkConfig));
    if (networkConfig == NULL) {
        char *message = "can not create network config instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    Json *configJson = parseJson(content);
    Json *trainBatchSizeJson = getJsonObjectItem(configJson, "trainBatchSize");
    if (trainBatchSizeJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist train batch size in the config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->trainBatchSize = (int)getJsonNumberValue(trainBatchSizeJson);

    Json *trainEpochCountJson = getJsonObjectItem(configJson, "trainEpochCount");
    if (trainEpochCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist train epoch count in the config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->trainEpochCount = (int)getJsonNumberValue(trainEpochCountJson);

    Json *learnRateJson = getJsonObjectItem(configJson, "learnRateValue");
    if (learnRateJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist learn rate config in the config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->learnRateValue = (float)getJsonNumberValue(learnRateJson);
    
    Json *inputConfigJson = getJsonObjectItem(configJson, "inputLayer");
    if (inputConfigJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist input layer configuration in the config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }

    networkConfig->inputLayerConfig.isOutputLayer = false;
    networkConfig->inputLayerConfig.activatorKind = EQUAL;
    Json *inputRowCountJson = getJsonObjectItem(inputConfigJson, "rowCount");
    if (inputRowCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist row count configuration in input layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->inputLayerConfig.matrixRowCount = (int)getJsonNumberValue(inputRowCountJson);

    Json *inputColumnCountJson = getJsonObjectItem(inputConfigJson, "columnCount");
    if (inputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist column count configuration in input layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->inputLayerConfig.matrixColumnCount = (int)getJsonNumberValue(inputColumnCountJson);
    
    Json *inputDimensionJson = getJsonObjectItem(inputConfigJson, "dimension");
    if (inputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist dimension configuration in input layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->inputLayerConfig.biasDimensionCount = (int)getJsonNumberValue(inputDimensionJson);

    Json *outputConfigJson = getJsonObjectItem(configJson, "outputLayer");
    if (outputConfigJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist output layer configuration in the config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }

    networkConfig->outputLayerConfig.isOutputLayer = true;
    networkConfig->outputLayerConfig.activatorKind = SOFTMAX;
    networkConfig->outputLayerConfig.activatorLossKind = SOFTMAX_CEL;
    Json *outputRowCountJson = getJsonObjectItem(outputConfigJson, "rowCount");
    if (outputRowCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist row count configuration in output layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->outputLayerConfig.matrixRowCount = (int)getJsonNumberValue(outputRowCountJson);

    Json *outputColumnCountJson = getJsonObjectItem(outputConfigJson, "columnCount");
    if (outputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist column count configuration in output layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->outputLayerConfig.matrixColumnCount = (int)getJsonNumberValue(outputColumnCountJson);

    Json *outputDimensionJson = getJsonObjectItem(outputConfigJson, "dimension");
    if (outputDimensionJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist dimension configuration in output layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->outputLayerConfig.biasDimensionCount = (int)getJsonNumberValue(outputDimensionJson);
    
    Json *hiddenConfigJsonList = getJsonObjectItem(configJson, "hiddenLayers");
    if (hiddenConfigJsonList == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist hidden layer configuration in config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }

    int count = getJsonArraySize(hiddenConfigJsonList);

    networkConfig->hiddenLayerConfigCount = count;
    networkConfig->hiddenLayerConfigList = (LayerConfig *)allocate(sizeof(LayerConfig)*count);
    if (networkConfig->hiddenLayerConfigList == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "can not initialize hidden layer config list for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<count;++i) {
        Json *hiddenConfigJson = getJsonArrayItem(hiddenConfigJsonList, i);
        networkConfig->hiddenLayerConfigList[i].isOutputLayer = false;
        networkConfig->hiddenLayerConfigList[i].activatorKind = EQUAL;
        Json *hiddenRowCountJson = getJsonObjectItem(hiddenConfigJson, "rowCount");
        if (hiddenRowCountJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);
            char *message = "there does not exist row count configuration in hidden layer configuration^o^";
            return createResultWithoutData(CONFIG_NO_EXIST, message);
        }
        networkConfig->hiddenLayerConfigList[i].matrixRowCount = (int)getJsonNumberValue(hiddenRowCountJson);

        Json *hiddenColumnCountJson = getJsonObjectItem(hiddenConfigJson, "columnCount");
        if (hiddenColumnCountJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);
            char *message = "there does not exist column count configuration in hidden layer configuration^o^";
            return createResultWithoutData(CONFIG_NO_EXIST, message);
        }
        networkConfig->hiddenLayerConfigList[i].matrixColumnCount = (int)getJsonNumberValue(hiddenColumnCountJson);

        Json *hiddenDimensionJson = getJsonObjectItem(hiddenConfigJson, "dimension");
        if (hiddenDimensionJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);
            char *message = "there does not exist dimension configuration in hidden layer configuration^o^";
            return createResultWithoutData(CONFIG_NO_EXIST, message);
        }
        networkConfig->hiddenLayerConfigList[i].biasDimensionCount = (int)getJsonNumberValue(hiddenDimensionJson);
    }

    deleteJson(configJson);
    return createResultWithData(SUCCESS, NULL, TYPE_NETWORK_CONFIG, networkConfig);
}