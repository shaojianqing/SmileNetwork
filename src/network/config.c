#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"
#include "../dataset/train.h"
#include "../file/file.h"
#include "../json/json.h"

#include "activator.h"
#include "bias.h"
#include "matrix.h"
#include "vector.h"
#include "loss.h"
#include "config.h"
#include "linear.h"

struct LinearLayerConfig {

    bool isOutputLayer;

    int matrixRowCount;

    int matrixColumnCount;

    int biasDimensionCount;

    ActivatorKind activatorKind;

    ActivatorLossKind activatorLossKind;
};

struct ConvLayerConfig{

    int fieldSize;

    int channelCount;
};

struct PoolLayerConfig{
    
    int poolSize;

    int poolStride;
};

struct DeepNetworkConfig {

    int trainBatchSize;

    int trainEpochCount;

    float learnRateValue;

    int hiddenLayerConfigCount;

    LinearLayerConfig *inputLayerConfig;

    LinearLayerConfig *outputLayerConfig;

    LinearLayerConfig **hiddenLayerConfigList;
};

struct ConvNetworkConfig {

    int trainBatchSize;

    int trainEpochCount;

    float learnRateValue;
};

static Exception FileOperateException = {FileOperateExceptionType};
static Exception MemoryAllocException = {MemoryAllocExceptionType};
static Exception ConfigErrorException = {ConfigErrorExceptionType};

static DeepNetworkConfig* parseConfig(char *content);

DeepNetworkConfig* loadNetworkConfig(char *filepath) {
    File *configFile = openFile(filepath, O_RDONLY);
    if (configFile == NULL) {
        throw(&FileOperateException, "config open network configuration file error for existence or permission reason");
    }

    char *content = readCharString(configFile);
    closeFile(configFile);

    DeepNetworkConfig *networkConfig = parseConfig(content);
    release(content);
    
    return networkConfig;
}

void releaseDeepNetworkConfig(DeepNetworkConfig *config) {
    if (config != NULL) {
        for (int i=0;i<config->hiddenLayerConfigCount;++i) {
            LinearLayerConfig *hiddenLayerConfig = config->hiddenLayerConfigList[i];
            release(hiddenLayerConfig);
        }
        release(config->inputLayerConfig);
        release(config->outputLayerConfig);
        release(config->hiddenLayerConfigList);
        release(config);
    }
}

static DeepNetworkConfig* createAndInitialize() {
    DeepNetworkConfig *networkConfig = (DeepNetworkConfig *)allocate(sizeof(DeepNetworkConfig));
    if (networkConfig == NULL) {
        return NULL;
    }

    networkConfig->inputLayerConfig = (LinearLayerConfig *)allocate(sizeof(LinearLayerConfig));
    if (networkConfig->inputLayerConfig == NULL) {
        release(networkConfig);
        return NULL;
    }

    networkConfig->outputLayerConfig = (LinearLayerConfig *)allocate(sizeof(LinearLayerConfig));
    if (networkConfig->outputLayerConfig == NULL) {
        release(networkConfig);
        release(networkConfig->outputLayerConfig);
        return NULL;
    }

    return networkConfig;
}

static DeepNetworkConfig* initializeHiddenLayerConfig(DeepNetworkConfig* config, int hiddenLayerCount) {
    config->hiddenLayerConfigCount = hiddenLayerCount;
    config->hiddenLayerConfigList = (LinearLayerConfig**)allocate(sizeof(LinearLayerConfig*) * hiddenLayerCount);
    for (int i=0;i<hiddenLayerCount;++i) {
        config->hiddenLayerConfigList[i] = (LinearLayerConfig*)allocate(sizeof(LinearLayerConfig));
    }
    return config;
}

static DeepNetworkConfig* parseConfig(char *content) {
    DeepNetworkConfig *networkConfig = createAndInitialize();
    if (networkConfig == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for network config!");
    }

    Json *configJson = parseJson(content);
    Json *trainBatchSizeJson = getJsonObjectItem(configJson, "trainBatchSize");
    if (trainBatchSizeJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist train batch size in the config file!");
    }
    networkConfig->trainBatchSize = (int)getJsonNumberValue(trainBatchSizeJson);

    Json *trainEpochCountJson = getJsonObjectItem(configJson, "trainEpochCount");
    if (trainEpochCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist train epoch count in the config file!");
    }
    networkConfig->trainEpochCount = (int)getJsonNumberValue(trainEpochCountJson);

    Json *learnRateJson = getJsonObjectItem(configJson, "learnRateValue");
    if (learnRateJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist learn rate config in the config file!");
    }
    networkConfig->learnRateValue = (float)getJsonNumberValue(learnRateJson);
    
    Json *inputConfigJson = getJsonObjectItem(configJson, "inputLayer");
    if (inputConfigJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist input layer configuration in the config file!");
    }

    networkConfig->inputLayerConfig->isOutputLayer = false;
    networkConfig->inputLayerConfig->activatorKind = RELU;
    Json *inputRowCountJson = getJsonObjectItem(inputConfigJson, "rowCount");
    if (inputRowCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist row count configuration in input layer configuration!");
    }
    networkConfig->inputLayerConfig->matrixRowCount = (int)getJsonNumberValue(inputRowCountJson);

    Json *inputColumnCountJson = getJsonObjectItem(inputConfigJson, "columnCount");
    if (inputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist column count configuration in input layer configuration!");
    }
    networkConfig->inputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(inputColumnCountJson);
    
    Json *inputDimensionJson = getJsonObjectItem(inputConfigJson, "dimension");
    if (inputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist dimension configuration in input layer configuration!");
    }
    networkConfig->inputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(inputDimensionJson);

    Json *outputConfigJson = getJsonObjectItem(configJson, "outputLayer");
    if (outputConfigJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist output layer configuration in the config file!");
    }

    networkConfig->outputLayerConfig->isOutputLayer = true;
    networkConfig->outputLayerConfig->activatorKind = SOFTMAX;
    networkConfig->outputLayerConfig->activatorLossKind = SOFTMAX_CEL;
    Json *outputRowCountJson = getJsonObjectItem(outputConfigJson, "rowCount");
    if (outputRowCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist row count configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->matrixRowCount = (int)getJsonNumberValue(outputRowCountJson);

    Json *outputColumnCountJson = getJsonObjectItem(outputConfigJson, "columnCount");
    if (outputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist column count configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(outputColumnCountJson);

    Json *outputDimensionJson = getJsonObjectItem(outputConfigJson, "dimension");
    if (outputDimensionJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist dimension configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(outputDimensionJson);
    
    Json *hiddenConfigJsonList = getJsonObjectItem(configJson, "hiddenLayers");
    if (hiddenConfigJsonList == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist hidden layer configuration in config file!");
    }

    int count = getJsonArraySize(hiddenConfigJsonList);

    networkConfig = initializeHiddenLayerConfig(networkConfig, count);
    if (networkConfig->hiddenLayerConfigList == NULL) {
        release(networkConfig);
        deleteJson(configJson);

        throw(&MemoryAllocException, "can not allocate memory for hidden layer config!");
    }

    for (int i=0;i<count;++i) {
        Json *hiddenConfigJson = getJsonArrayItem(hiddenConfigJsonList, i);
        networkConfig->hiddenLayerConfigList[i]->isOutputLayer = false;
        networkConfig->hiddenLayerConfigList[i]->activatorKind = RELU;
        Json *hiddenRowCountJson = getJsonObjectItem(hiddenConfigJson, "rowCount");
        if (hiddenRowCountJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist row count configuration in hidden layer configuration!");
        }
        networkConfig->hiddenLayerConfigList[i]->matrixRowCount = (int)getJsonNumberValue(hiddenRowCountJson);

        Json *hiddenColumnCountJson = getJsonObjectItem(hiddenConfigJson, "columnCount");
        if (hiddenColumnCountJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist column count configuration in hidden layer configuration!");
        }
        networkConfig->hiddenLayerConfigList[i]->matrixColumnCount = (int)getJsonNumberValue(hiddenColumnCountJson);

        Json *hiddenDimensionJson = getJsonObjectItem(hiddenConfigJson, "dimension");
        if (hiddenDimensionJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist dimension configuration in hidden layer configuration!");
        }
        networkConfig->hiddenLayerConfigList[i]->biasDimensionCount = (int)getJsonNumberValue(hiddenDimensionJson);
    }

    deleteJson(configJson);
    return networkConfig;
}

int getTrainConfigBatchSize(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->trainBatchSize;
    }
    return 0;
}

int getTrainConfigEpochCount(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->trainEpochCount;
    }
    return 0;
}

float getLearnRateConfigValue(DeepNetworkConfig *config) {
        if (config != NULL) {
        return config->learnRateValue;
    }
    return 0.0;
}

int getHiddenLayerConfigCount(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigCount;
    }
    return 0;
}

LinearLayerConfig* getInputLayerConfig(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->inputLayerConfig;
    }
    return NULL;
}

LinearLayerConfig* getOutputLayerConfig(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->outputLayerConfig;
    }
    return NULL;
}

LinearLayerConfig** getHiddenLayerConfigList(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigList;
    }
    return NULL;
}

bool isOutputLayer(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->isOutputLayer;
    }
    return false;
}

int getMatrixConfigRowCount(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->matrixRowCount;
    }
    return 0;
}

int getMatrixConfigColumnCount(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->matrixColumnCount;
    }
    return 0;
}

int getBiasConfigDimensionCount(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->biasDimensionCount;
    }
    return 0;
}

ActivatorKind getConfigActivatorKind(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->activatorKind;
    }
    return 0;
}

ActivatorLossKind getConfigActivatorLossKind(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->activatorLossKind;
    }
    return 0;
}