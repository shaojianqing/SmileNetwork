#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../datatype/stringtype.h"
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

struct ConvLayerConfig {

    String *name;

    String *type;

    int fieldSize;

    int kernelCount;

    int channelCount;

    int paddingSize;

    int poolSize;

    int poolStride;

    String *nextName;

    String *prevName;
};

struct ConvOutputLayerConfig {

    String *name;

    String *type;

    int matrixRowCount;

    int matrixColumnCount;

    int biasDimensionCount;

    ActivatorKind activatorKind;

    ActivatorLossKind activatorLossKind;

    String *prevName;
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

    int hiddenLayerConfigCount;

    ConvLayerConfig *inputLayerConfig;

    ConvOutputLayerConfig *outputLayerConfig;

    ConvLayerConfig **hiddenLayerConfigs;
};

static Exception FileOperateException = {FileOperateExceptionType};
static Exception MemoryAllocException = {MemoryAllocExceptionType};
static Exception ConfigErrorException = {ConfigErrorExceptionType};

static DeepNetworkConfig* createAndInitializeDeepNetworkConfig();

static ConvNetworkConfig* createAndInitializeConvNetworkConfig();

static void parseDeepParameter(DeepNetworkConfig* networkConfig, Json *configJson);

static void parseDeepInputLayer(DeepNetworkConfig* networkConfig, Json *configJson);

static void parseDeepOutputLayer(DeepNetworkConfig* networkConfig, Json *configJson);

static void parseDeepHiddenLayers(DeepNetworkConfig* networkConfig, Json *configJson);

static void parseConvParameter(ConvNetworkConfig* networkConfig, Json *configJson);

static void parseConvInputLayer(ConvNetworkConfig* networkConfig, Json *configJson);

static void parseConvOutputLayer(ConvNetworkConfig* networkConfig, Json *configJson);

static void parseConvHiddenLayers(ConvNetworkConfig* networkConfig, Json *configJson);

Json* loadJsonConfigData(char *filepath) {
    File *configFile = openFile(filepath, O_RDONLY);
    if (configFile == NULL) {
        throw(&FileOperateException, "config open network configuration file error for existence or permission reason");
    }

    char *content = readCharString(configFile);
    closeFile(configFile);

    Json *configJson = parseJson(content);
    release(content);

    return configJson;
}

char* getNetworkConfigType(Json *configJson) {
   
    Json *networkTypeItem = getJsonObjectItem(configJson, "networkType");
    if (networkTypeItem == NULL) {
        release(configJson);

        throw(&ConfigErrorException, "there does not exist network type in the config file!");
    }

    return getJsonStringValue(networkTypeItem);
}

DeepNetworkConfig* loadDeepNetworkConfig(Json *configJson) {
    DeepNetworkConfig *networkConfig = createAndInitializeDeepNetworkConfig();
    if (networkConfig == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for deep neural network config!");
    }

    parseDeepParameter(networkConfig, configJson);
    parseDeepInputLayer(networkConfig, configJson);
    parseDeepOutputLayer(networkConfig, configJson);
    parseDeepHiddenLayers(networkConfig, configJson);

    deleteJson(configJson);
    return networkConfig;
}

ConvNetworkConfig* loadConvNetworkConfig(Json *configJson) {
    ConvNetworkConfig *networkConfig = createAndInitializeConvNetworkConfig();
    if (networkConfig == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for convolution neural network config!");
    }

    parseConvParameter(networkConfig, configJson);
    parseConvInputLayer(networkConfig, configJson);
    parseConvOutputLayer(networkConfig, configJson);
    parseConvHiddenLayers(networkConfig, configJson);

    deleteJson(configJson);
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

void releaseConvNetworkConfig(ConvNetworkConfig *config) {

}

// the below part is specific for deep neural network configuration initialization
static DeepNetworkConfig* createAndInitializeDeepNetworkConfig() {
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

static void parseDeepParameter(DeepNetworkConfig* networkConfig, Json *configJson) {
    Json *trainBatchSizeJson = getJsonObjectItem(configJson, "trainBatchSize");
    if (trainBatchSizeJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist train batch size in the config file!");
    }
    networkConfig->trainBatchSize = (int)getJsonNumberValue(trainBatchSizeJson);

    Json *trainEpochCountJson = getJsonObjectItem(configJson, "trainEpochCount");
    if (trainEpochCountJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist train epoch count in the config file!");
    }
    networkConfig->trainEpochCount = (int)getJsonNumberValue(trainEpochCountJson);

    Json *learnRateJson = getJsonObjectItem(configJson, "learnRateValue");
    if (learnRateJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist learn rate config in the config file!");
    }
    networkConfig->learnRateValue = (float)getJsonNumberValue(learnRateJson);
}

static void parseDeepInputLayer(DeepNetworkConfig* networkConfig, Json *configJson) {
    Json *inputConfigJson = getJsonObjectItem(configJson, "inputLayer");
    if (inputConfigJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist input layer configuration in the config file!");
    }

    networkConfig->inputLayerConfig->isOutputLayer = false;
    networkConfig->inputLayerConfig->activatorKind = RELU;
    Json *inputRowCountJson = getJsonObjectItem(inputConfigJson, "rowCount");
    if (inputRowCountJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist row count configuration in input layer configuration!");
    }
    networkConfig->inputLayerConfig->matrixRowCount = (int)getJsonNumberValue(inputRowCountJson);

    Json *inputColumnCountJson = getJsonObjectItem(inputConfigJson, "columnCount");
    if (inputColumnCountJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist column count configuration in input layer configuration!");
    }
    networkConfig->inputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(inputColumnCountJson);
    
    Json *inputDimensionJson = getJsonObjectItem(inputConfigJson, "dimension");
    if (inputColumnCountJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist dimension configuration in input layer configuration!");
    }
    networkConfig->inputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(inputDimensionJson);
}

static void parseDeepOutputLayer(DeepNetworkConfig* networkConfig, Json *configJson) {
    Json *outputConfigJson = getJsonObjectItem(configJson, "outputLayer");
    if (outputConfigJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist output layer configuration in the config file!");
    }

    networkConfig->outputLayerConfig->isOutputLayer = true;
    networkConfig->outputLayerConfig->activatorKind = SOFTMAX;
    networkConfig->outputLayerConfig->activatorLossKind = SOFTMAX_CEL;
    Json *outputRowCountJson = getJsonObjectItem(outputConfigJson, "rowCount");
    if (outputRowCountJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist row count configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->matrixRowCount = (int)getJsonNumberValue(outputRowCountJson);

    Json *outputColumnCountJson = getJsonObjectItem(outputConfigJson, "columnCount");
    if (outputColumnCountJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist column count configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(outputColumnCountJson);

    Json *outputDimensionJson = getJsonObjectItem(outputConfigJson, "dimension");
    if (outputDimensionJson == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist dimension configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(outputDimensionJson);
}

static void parseDeepHiddenLayers(DeepNetworkConfig* networkConfig, Json *configJson) {
    Json *hiddenConfigJsonList = getJsonObjectItem(configJson, "hiddenLayers");
    if (hiddenConfigJsonList == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist hidden layer configuration in config file!");
    }

    int count = getJsonArraySize(hiddenConfigJsonList);

    networkConfig = initializeHiddenLayerConfig(networkConfig, count);
    if (networkConfig->hiddenLayerConfigList == NULL) {
        releaseDeepNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&MemoryAllocException, "can not allocate memory for hidden layer config!");
    }

    for (int i=0;i<count;++i) {
        Json *hiddenConfigJson = getJsonArrayItem(hiddenConfigJsonList, i);
        networkConfig->hiddenLayerConfigList[i]->isOutputLayer = false;
        networkConfig->hiddenLayerConfigList[i]->activatorKind = RELU;
        Json *hiddenRowCountJson = getJsonObjectItem(hiddenConfigJson, "rowCount");
        if (hiddenRowCountJson == NULL) {
            releaseDeepNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist row count configuration in hidden layer configuration!");
        }
        networkConfig->hiddenLayerConfigList[i]->matrixRowCount = (int)getJsonNumberValue(hiddenRowCountJson);

        Json *hiddenColumnCountJson = getJsonObjectItem(hiddenConfigJson, "columnCount");
        if (hiddenColumnCountJson == NULL) {
            releaseDeepNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist column count configuration in hidden layer configuration!");
        }
        networkConfig->hiddenLayerConfigList[i]->matrixColumnCount = (int)getJsonNumberValue(hiddenColumnCountJson);

        Json *hiddenDimensionJson = getJsonObjectItem(hiddenConfigJson, "dimension");
        if (hiddenDimensionJson == NULL) {
            releaseDeepNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist dimension configuration in hidden layer configuration!");
        }
        networkConfig->hiddenLayerConfigList[i]->biasDimensionCount = (int)getJsonNumberValue(hiddenDimensionJson);
    }
}

// the below part is specific for convolution neural network configuration initialization
static ConvNetworkConfig* createAndInitializeConvNetworkConfig() {
    ConvNetworkConfig *networkConfig = (ConvNetworkConfig *)allocate(sizeof(ConvNetworkConfig));
    if (networkConfig == NULL) {
        return NULL;
    }

    networkConfig->inputLayerConfig = (ConvLayerConfig *)allocate(sizeof(ConvLayerConfig));
    if (networkConfig->inputLayerConfig == NULL) {
        release(networkConfig);
        return NULL;
    }

    networkConfig->outputLayerConfig = (ConvOutputLayerConfig *)allocate(sizeof(ConvOutputLayerConfig));
    if (networkConfig->outputLayerConfig == NULL) {
        release(networkConfig->inputLayerConfig);
        release(networkConfig);
        return NULL;
    }

    return networkConfig;
}

static ConvNetworkConfig* initializeLayerConfigList(ConvNetworkConfig *networkConfig, int hiddenLayerConfigCount) {
    networkConfig->hiddenLayerConfigCount = hiddenLayerConfigCount;
    networkConfig->hiddenLayerConfigs = (ConvLayerConfig**)allocate(sizeof(ConvLayerConfig*)*hiddenLayerConfigCount);
    for (int i=0;i<hiddenLayerConfigCount;++i) {
        networkConfig->hiddenLayerConfigs[i] = (ConvLayerConfig*)allocate(sizeof(ConvLayerConfig));
    }
    return networkConfig;
}

static void parseConvParameter(ConvNetworkConfig* networkConfig, Json *configJson) {
    if (networkConfig == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for convolution neural network config!");
    }

    Json *trainBatchSizeJson = getJsonObjectItem(configJson, "trainBatchSize");
    if (trainBatchSizeJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist train batch size in the config file!");
    }
    networkConfig->trainBatchSize = (int)getJsonNumberValue(trainBatchSizeJson);

    Json *trainEpochCountJson = getJsonObjectItem(configJson, "trainEpochCount");
    if (trainEpochCountJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist train epoch count in the config file!");
    }
    networkConfig->trainEpochCount = (int)getJsonNumberValue(trainEpochCountJson);

    Json *learnRateJson = getJsonObjectItem(configJson, "learnRateValue");
    if (learnRateJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist learn rate config in the config file!");
    }
    networkConfig->learnRateValue = (float)getJsonNumberValue(learnRateJson);
}

static void parseConvInputLayer(ConvNetworkConfig* networkConfig, Json *configJson) {
    Json *inputLayerJson = getJsonObjectItem(configJson, "inputLayer");
    if (inputLayerJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist input layer config in the config file!");
    }

    Json *nameJson = getJsonObjectItem(inputLayerJson, "name");
    if (nameJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist name config item in the config file!");
    }

    char *name = getJsonStringValue(nameJson);
    networkConfig->inputLayerConfig->name = createString(name);

    Json *typeJson = getJsonObjectItem(inputLayerJson, "type");
    if (typeJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist type config item in the config file!");
    }

    char *type = getJsonStringValue(typeJson);
    networkConfig->inputLayerConfig->type = createString(type);

    Json *fieldSizeJson = getJsonObjectItem(inputLayerJson, "fieldSize");
    if (fieldSizeJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist field size config item in the config file!");
    }
    int fieldSize = getJsonNumberValue(fieldSizeJson);
    networkConfig->inputLayerConfig->fieldSize = fieldSize;

    Json *paddingSizeJson = getJsonObjectItem(inputLayerJson, "paddingSize");
    if (paddingSizeJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist padding size config item in the config file!");
    }

    int paddingSize = getJsonNumberValue(paddingSizeJson);
    networkConfig->inputLayerConfig->paddingSize = paddingSize;

    Json *kernelCountJson = getJsonObjectItem(inputLayerJson, "kernelCount");
    if (kernelCountJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist kernel count config item in the config file!");
    }

    int kernelCount = getJsonNumberValue(kernelCountJson);
    networkConfig->inputLayerConfig->kernelCount = kernelCount;

    Json *channelCountJson = getJsonObjectItem(inputLayerJson, "channelCount");
    if (channelCountJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist channel count config item in the config file!");
    }

    int channelCount = getJsonNumberValue(channelCountJson);
    networkConfig->inputLayerConfig->channelCount = channelCount;
}

static void parseConvOutputLayer(ConvNetworkConfig* networkConfig, Json *configJson) {
    Json *outputLayerJson = getJsonObjectItem(configJson, "outputLayer");
    if (outputLayerJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist output layer config in the config file!");
    }

    networkConfig->outputLayerConfig->activatorKind = SOFTMAX;
    networkConfig->outputLayerConfig->activatorLossKind = SOFTMAX_CEL;

    Json *nameJson = getJsonObjectItem(outputLayerJson, "name");
    if (nameJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist name config item in the config file!");
    }

    char *name = getJsonStringValue(nameJson);
    networkConfig->outputLayerConfig->name = createString(name);

    Json *typeJson = getJsonObjectItem(outputLayerJson, "type");
    if (typeJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist type config item in the config file!");
    }

    char *type = getJsonStringValue(typeJson);
    networkConfig->outputLayerConfig->type = createString(type);

    Json *outputRowCountJson = getJsonObjectItem(outputLayerJson, "rowCount");
    if (outputRowCountJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist row count configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->matrixRowCount = (int)getJsonNumberValue(outputRowCountJson);

    Json *outputColumnCountJson = getJsonObjectItem(outputLayerJson, "columnCount");
    if (outputColumnCountJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist column count configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(outputColumnCountJson);

    Json *outputDimensionJson = getJsonObjectItem(outputLayerJson, "dimension");
    if (outputDimensionJson == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist dimension configuration in output layer configuration!");
    }
    networkConfig->outputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(outputDimensionJson);

    Json *prevNameJson = getJsonObjectItem(outputLayerJson, "prevName");
    if (prevNameJson != NULL) {
        char *prevName = getJsonStringValue(prevNameJson);
        networkConfig->outputLayerConfig->prevName = createString(prevName);
    }
}

static void parseConvHiddenLayers(ConvNetworkConfig* networkConfig, Json *configJson) {
    Json *hiddenLayerConfigList = getJsonObjectItem(configJson, "hiddenLayers");
    if (hiddenLayerConfigList == NULL) {
        releaseConvNetworkConfig(networkConfig);
        deleteJson(configJson);

        throw(&ConfigErrorException, "there does not exist layer configuration list in config file!");
    }

    int count = getJsonArraySize(hiddenLayerConfigList);
    networkConfig = initializeLayerConfigList(networkConfig, count);
    for (int i=0;i<count;++i) {
        Json *layerConfigJson = getJsonArrayItem(hiddenLayerConfigList, i);

        Json *nameJson = getJsonObjectItem(layerConfigJson, "name");
        if (nameJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist name config item in the config file!");
        }

        char *name = getJsonStringValue(nameJson);
        networkConfig->hiddenLayerConfigs[i]->name = createString(name);

        Json *typeJson = getJsonObjectItem(layerConfigJson, "type");
        if (typeJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist type config item in the config file!");
        }

        char *type = getJsonStringValue(typeJson);
        networkConfig->hiddenLayerConfigs[i]->type = createString(type);

        Json *fieldSizeJson = getJsonObjectItem(layerConfigJson, "fieldSize");
        if (fieldSizeJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist field size config item in the config file!");
        }

        int fieldSize = getJsonNumberValue(fieldSizeJson);
        networkConfig->hiddenLayerConfigs[i]->fieldSize = fieldSize;

        Json *paddingSizeJson = getJsonObjectItem(layerConfigJson, "paddingSize");
        if (paddingSizeJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist padding size config item in the config file!");
        }

        int paddingSize = getJsonNumberValue(paddingSizeJson);
        networkConfig->hiddenLayerConfigs[i]->paddingSize = paddingSize;


        Json *kernelCountJson = getJsonObjectItem(layerConfigJson, "kernelCount");
        if (kernelCountJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist kernel count config item in the config file!");
        }

        int kernelCount = getJsonNumberValue(kernelCountJson);
        networkConfig->hiddenLayerConfigs[i]->kernelCount = kernelCount;

        Json *channelCountJson = getJsonObjectItem(layerConfigJson, "channelCount");
        if (channelCountJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist channel count config item in the config file!");
        }

        int channelCount = getJsonNumberValue(channelCountJson);
        networkConfig->hiddenLayerConfigs[i]->channelCount = channelCount;

        Json *poolSizeJson = getJsonObjectItem(layerConfigJson, "poolSize");
        if (poolSizeJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist pool size config item in the config file!");
        }

        int poolSize = getJsonNumberValue(poolSizeJson);
        networkConfig->hiddenLayerConfigs[i]->poolSize = poolSize;

        Json *poolStrideJson = getJsonObjectItem(layerConfigJson, "poolStride");
        if (poolStrideJson == NULL) {
            releaseConvNetworkConfig(networkConfig);
            deleteJson(configJson);

            throw(&ConfigErrorException, "there does not exist pool stride config item in the config file!");
        }

        int poolStride = getJsonNumberValue(poolStrideJson);
        networkConfig->hiddenLayerConfigs[i]->poolStride = poolStride;

        Json *prevNameJson = getJsonObjectItem(layerConfigJson, "prevName");
        if (prevNameJson != NULL) {
            char *prevName = getJsonStringValue(prevNameJson);
            networkConfig->hiddenLayerConfigs[i]->prevName = createString(prevName);
        }

        Json *nextNameJson = getJsonObjectItem(layerConfigJson, "nextName");
        if (nextNameJson != NULL) {
            char *nextName = getJsonStringValue(nextNameJson);
            networkConfig->hiddenLayerConfigs[i]->nextName = createString(nextName);
        }
    }
}

int getDeepTrainConfigBatchSize(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->trainBatchSize;
    }
    return 0;
}

int getDeepTrainConfigEpochCount(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->trainEpochCount;
    }
    return 0;
}

float getDeepLearnRateConfigValue(DeepNetworkConfig *config) {
        if (config != NULL) {
        return config->learnRateValue;
    }
    return 0.0;
}

int getDeepHiddenLayerConfigCount(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigCount;
    }
    return 0;
}

LinearLayerConfig* getDeepInputLayerConfig(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->inputLayerConfig;
    }
    return NULL;
}

LinearLayerConfig* getDeepOutputLayerConfig(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->outputLayerConfig;
    }
    return NULL;
}

LinearLayerConfig** getDeepHiddenLayerConfigList(DeepNetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigList;
    }
    return NULL;
}

bool isLinearOutputLayer(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->isOutputLayer;
    }
    return false;
}

int getLinearMatrixConfigRowCount(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->matrixRowCount;
    }
    return 0;
}

int getLinearMatrixConfigColumnCount(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->matrixColumnCount;
    }
    return 0;
}

int getLinearBiasConfigDimensionCount(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->biasDimensionCount;
    }
    return 0;
}

ActivatorKind getLinearConfigActivatorKind(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->activatorKind;
    }
    return 0;
}

ActivatorLossKind getLinearConfigActivatorLossKind(LinearLayerConfig *config) {
    if (config != NULL) {
        return config->activatorLossKind;
    }
    return 0;
}

int getConvTrainConfigBatchSize(ConvNetworkConfig *config) {
    if (config != NULL) {
        return config->trainBatchSize;
    }
    return 0;
}

int getConvTrainConfigEpochCount(ConvNetworkConfig *config) {
    if (config != NULL) {
        return config->trainEpochCount;
    }
    return 0;
}

float getConvLearnRateConfigValue(ConvNetworkConfig *config) {
        if (config != NULL) {
        return config->learnRateValue;
    }
    return 0.0;
}

int getConvHiddenLayerConfigCount(ConvNetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigCount;
    }
    return 0;
}

ConvLayerConfig* getConvInputLayerConfig(ConvNetworkConfig *config) {
    if (config != NULL) {
        return config->inputLayerConfig;
    }
    return NULL;
}

ConvOutputLayerConfig* getConvOutputLayerConfig(ConvNetworkConfig *config) {
    if (config != NULL) {
        return config->outputLayerConfig;
    }
    return NULL;
}

ConvLayerConfig** getConvHiddenLayerConfigList(ConvNetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigs;
    }
    return NULL;
}

String* getConvLayerName(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->name;
    }
    return NULL;
}

String* getConvLayerType(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->type;
    }
    return NULL;
}

int getConvLayerFieldSize(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->fieldSize;
    }
    return 0;
}

int getConvLayerKernelCount(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->kernelCount;
    }
    return 0;
}

int getConvLayerChannelCount(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->channelCount;
    }
    return 0;
}

int getConvLayerPaddingSize(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->paddingSize;
    }
    return 0;
}

int getConvLayerPoolSize(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->poolSize;
    }
    return 0;
}

int getConvLayerPoolStride(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->poolStride;
    }
    return 0;
}

String* getConvLayerPrevName(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->prevName;
    }
    return NULL;
}

String* getConvLayerNextName(ConvLayerConfig* config) {
    if (config != NULL) {
        return config->nextName;
    }
    return NULL;
}

String* getConvOutputLayerPrevName(ConvOutputLayerConfig* config) {
    if (config != NULL) {
        return config->prevName;
    }
    return NULL;
}

int getConvOutputMatrixConfigRowCount(ConvOutputLayerConfig *config) {
    if (config != NULL) {
        return config->matrixRowCount;
    }
    return 0;
}

int getConvOutputMatrixConfigColumnCount(ConvOutputLayerConfig *config) {
    if (config != NULL) {
        return config->matrixColumnCount;
    }
    return 0;
}

int getConvOutputBiasConfigDimensionCount(ConvOutputLayerConfig *config) {
    if (config != NULL) {
        return config->biasDimensionCount;
    }
    return 0;
}

ActivatorKind getConvOutputConfigActivatorKind(ConvOutputLayerConfig *config) {
    if (config != NULL) {
        return config->activatorKind;
    }
    return 0;
}

ActivatorLossKind getConvOutputConfigActivatorLossKind(ConvOutputLayerConfig *config) {
    if (config != NULL) {
        return config->activatorLossKind;
    }
    return 0;
}