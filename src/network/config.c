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
#include "config.h"
#include "layer.h"

struct LayerConfig {

    bool isOutputLayer;

    int matrixRowCount;

    int matrixColumnCount;

    int biasDimensionCount;

    ActivatorKind activatorKind;

    ActivatorLossKind activatorLossKind;
};

struct NetworkConfig {

    int trainBatchSize;

    int trainEpochCount;

    float learnRateValue;

    int hiddenLayerConfigCount;

    LayerConfig *inputLayerConfig;

    LayerConfig *outputLayerConfig;

    LayerConfig **hiddenLayerConfigList;
};

static Result* parseConfig(char *content);

Result *loadNetworkConfig(char *filepath) {
    File *configFile = openFile(filepath, O_RDONLY);
    if (configFile == NULL) {
        char *message = "config open network configuration file error for existence or permission reason^o^";
        createResultWithoutData(FILE_OPEN_ERROR, message);
    }

    Result *readResult = readCharString(configFile);
    if (!success(readResult)) {
        closeFile(configFile);
        return readResult;
    }
    char *content = (char*)getData(readResult);
    releaseResult(readResult);
    closeFile(configFile);

    Result *parseResult = parseConfig(content);
    release(content);
    
    return parseResult;
}

void releaseNetworkConfig(NetworkConfig *config) {
    if (config != NULL) {
        for (int i=0;i<config->hiddenLayerConfigCount;++i) {
            LayerConfig *hiddenLayerConfig = config->hiddenLayerConfigList[i];
            release(hiddenLayerConfig);
        }
        release(config->inputLayerConfig);
        release(config->outputLayerConfig);
        release(config->hiddenLayerConfigList);
        release(config);
    }
}

static NetworkConfig* createAndInitialize() {
    NetworkConfig *networkConfig = (NetworkConfig *)allocate(sizeof(NetworkConfig));
    if (networkConfig == NULL) {
        return NULL;
    }

    networkConfig->inputLayerConfig = (LayerConfig *)allocate(sizeof(LayerConfig));
    if (networkConfig->inputLayerConfig == NULL) {
        release(networkConfig);
        return NULL;
    }

    networkConfig->outputLayerConfig = (LayerConfig *)allocate(sizeof(LayerConfig));
    if (networkConfig->outputLayerConfig == NULL) {
        release(networkConfig);
        release(networkConfig->outputLayerConfig);
        return NULL;
    }

    return networkConfig;
}

static NetworkConfig* initializeHiddenLayerConfig(NetworkConfig* config, int hiddenLayerCount) {
    config->hiddenLayerConfigCount = hiddenLayerCount;
    config->hiddenLayerConfigList = (LayerConfig**)allocate(sizeof(LayerConfig*) * hiddenLayerCount);
    for (int i=0;i<hiddenLayerCount;++i) {
        config->hiddenLayerConfigList[i] = (LayerConfig*)allocate(sizeof(LayerConfig));
    }
    return config;
}

static Result* parseConfig(char *content) {
    NetworkConfig *networkConfig = createAndInitialize();
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

    networkConfig->inputLayerConfig->isOutputLayer = false;
    networkConfig->inputLayerConfig->activatorKind = RELU;
    Json *inputRowCountJson = getJsonObjectItem(inputConfigJson, "rowCount");
    if (inputRowCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist row count configuration in input layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->inputLayerConfig->matrixRowCount = (int)getJsonNumberValue(inputRowCountJson);

    Json *inputColumnCountJson = getJsonObjectItem(inputConfigJson, "columnCount");
    if (inputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist column count configuration in input layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->inputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(inputColumnCountJson);
    
    Json *inputDimensionJson = getJsonObjectItem(inputConfigJson, "dimension");
    if (inputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist dimension configuration in input layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->inputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(inputDimensionJson);

    Json *outputConfigJson = getJsonObjectItem(configJson, "outputLayer");
    if (outputConfigJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist output layer configuration in the config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }

    networkConfig->outputLayerConfig->isOutputLayer = true;
    networkConfig->outputLayerConfig->activatorKind = SOFTMAX;
    networkConfig->outputLayerConfig->activatorLossKind = SOFTMAX_CEL;
    Json *outputRowCountJson = getJsonObjectItem(outputConfigJson, "rowCount");
    if (outputRowCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist row count configuration in output layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->outputLayerConfig->matrixRowCount = (int)getJsonNumberValue(outputRowCountJson);

    Json *outputColumnCountJson = getJsonObjectItem(outputConfigJson, "columnCount");
    if (outputColumnCountJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist column count configuration in output layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->outputLayerConfig->matrixColumnCount = (int)getJsonNumberValue(outputColumnCountJson);

    Json *outputDimensionJson = getJsonObjectItem(outputConfigJson, "dimension");
    if (outputDimensionJson == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist dimension configuration in output layer configuration^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }
    networkConfig->outputLayerConfig->biasDimensionCount = (int)getJsonNumberValue(outputDimensionJson);
    
    Json *hiddenConfigJsonList = getJsonObjectItem(configJson, "hiddenLayers");
    if (hiddenConfigJsonList == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "there does not exist hidden layer configuration in config file^o^";
        return createResultWithoutData(CONFIG_NO_EXIST, message);
    }

    int count = getJsonArraySize(hiddenConfigJsonList);

    networkConfig = initializeHiddenLayerConfig(networkConfig, count);
    if (networkConfig->hiddenLayerConfigList == NULL) {
        release(networkConfig);
        deleteJson(configJson);
        char *message = "can not initialize hidden layer config list for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<count;++i) {
        Json *hiddenConfigJson = getJsonArrayItem(hiddenConfigJsonList, i);
        networkConfig->hiddenLayerConfigList[i]->isOutputLayer = false;
        networkConfig->hiddenLayerConfigList[i]->activatorKind = RELU;
        Json *hiddenRowCountJson = getJsonObjectItem(hiddenConfigJson, "rowCount");
        if (hiddenRowCountJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);
            char *message = "there does not exist row count configuration in hidden layer configuration^o^";
            return createResultWithoutData(CONFIG_NO_EXIST, message);
        }
        networkConfig->hiddenLayerConfigList[i]->matrixRowCount = (int)getJsonNumberValue(hiddenRowCountJson);

        Json *hiddenColumnCountJson = getJsonObjectItem(hiddenConfigJson, "columnCount");
        if (hiddenColumnCountJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);
            char *message = "there does not exist column count configuration in hidden layer configuration^o^";
            return createResultWithoutData(CONFIG_NO_EXIST, message);
        }
        networkConfig->hiddenLayerConfigList[i]->matrixColumnCount = (int)getJsonNumberValue(hiddenColumnCountJson);

        Json *hiddenDimensionJson = getJsonObjectItem(hiddenConfigJson, "dimension");
        if (hiddenDimensionJson == NULL) {
            release(networkConfig);
            deleteJson(configJson);
            char *message = "there does not exist dimension configuration in hidden layer configuration^o^";
            return createResultWithoutData(CONFIG_NO_EXIST, message);
        }
        networkConfig->hiddenLayerConfigList[i]->biasDimensionCount = (int)getJsonNumberValue(hiddenDimensionJson);
    }

    deleteJson(configJson);
    return createResultWithData(SUCCESS, NULL, TYPE_NETWORK_CONFIG, networkConfig);
}

int getTrainConfigBatchSize(NetworkConfig *config) {
    if (config != NULL) {
        return config->trainBatchSize;
    }
    return 0;
}

int getTrainConfigEpochCount(NetworkConfig *config) {
    if (config != NULL) {
        return config->trainEpochCount;
    }
    return 0;
}

float getLearnRateConfigValue(NetworkConfig *config) {
        if (config != NULL) {
        return config->learnRateValue;
    }
    return 0.0;
}

int getHiddenLayerConfigCount(NetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigCount;
    }
    return 0;
}

LayerConfig* getInputLayerConfig(NetworkConfig *config) {
    if (config != NULL) {
        return config->inputLayerConfig;
    }
    return NULL;
}

LayerConfig* getOutputLayerConfig(NetworkConfig *config) {
    if (config != NULL) {
        return config->outputLayerConfig;
    }
    return NULL;
}

LayerConfig** getHiddenLayerConfigList(NetworkConfig *config) {
    if (config != NULL) {
        return config->hiddenLayerConfigList;
    }
    return NULL;
}

bool isOutputLayer(LayerConfig *config) {
    if (config != NULL) {
        return config->isOutputLayer;
    }
    return false;
}

int getMatrixConfigRowCount(LayerConfig *config) {
    if (config != NULL) {
        return config->matrixRowCount;
    }
    return 0;
}

int getMatrixConfigColumnCount(LayerConfig *config) {
    if (config != NULL) {
        return config->matrixColumnCount;
    }
    return 0;
}

int getBiasConfigDimensionCount(LayerConfig *config) {
    if (config != NULL) {
        return config->biasDimensionCount;
    }
    return 0;
}

ActivatorKind getConfigActivatorKind(LayerConfig *config) {
    if (config != NULL) {
        return config->activatorKind;
    }
    return 0;
}

ActivatorLossKind getConfigActivatorLossKind(LayerConfig *config) {
    if (config != NULL) {
        return config->activatorLossKind;
    }
    return 0;
}