#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../logger/logger.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../datatype/datatype.h"
#include "../datatype/hashmap.h"
#include "../datatype/arraylist.h"
#include "../datatype/stringtype.h"
#include "../generator/generator.h"
#include "../dataset/train.h"
#include "../json/json.h"

#include "config.h"
#include "activator.h"
#include "bias.h"
#include "loss.h"
#include "vector.h"
#include "tensor.h"
#include "matrix.h"
#include "linear.h"
#include "conv.h"
#include "network.h"

#define INITIAL_CONV_LAYER_COUNT 10

struct DeepNeuralNetwork {

    int trainBatchSize;

    int trainEpochCount;

    float learnRateValue;

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    AffineLayer **hiddenLayerList;

    int hiddenLayerCount;
};

struct ConvNeuralNetwork {

    int trainBatchSize;

    int trainEpochCount;

    float learnRateValue;

    Map *tensorLayerMap;

    ConvLayer *inputLayer;

    ConvOutputLayer *outputLayer;
};

extern Logger logger;

static DeepNeuralNetwork *deepNeuralNetwork;

static ConvNeuralNetwork *convNeuralNetwork;

static int getLabelValueFromOneHot(Vector *vector);

static bool checkDeepNeuralNetwork(DeepNeuralNetwork *neuralNetwork);

static bool checkConvNeuralNetwork(ConvNeuralNetwork *neuralNetwork);

bool constructDeepNeuralNetwork(DeepNetworkConfig *config) {
    deepNeuralNetwork = (DeepNeuralNetwork*)allocate(sizeof(DeepNeuralNetwork));
    if (deepNeuralNetwork != NULL) {
        deepNeuralNetwork->trainBatchSize = getDeepTrainConfigBatchSize(config);
        deepNeuralNetwork->trainEpochCount = getDeepTrainConfigEpochCount(config);
        deepNeuralNetwork->learnRateValue = getDeepLearnRateConfigValue(config);

        LinearLayerConfig *inputLayerConfig = getDeepInputLayerConfig(config);
        deepNeuralNetwork->inputLayer = buildInputLayer(inputLayerConfig);

        LinearLayerConfig *outputLayerConfig = getDeepOutputLayerConfig(config);
        deepNeuralNetwork->outputLayer = buildOutputLayer(outputLayerConfig);

        int hiddenLayerCount = getDeepHiddenLayerConfigCount(config);
        if (hiddenLayerCount > 0) {
            deepNeuralNetwork->hiddenLayerCount = hiddenLayerCount;
            deepNeuralNetwork->hiddenLayerList = (AffineLayer **)allocate(hiddenLayerCount * sizeof(AffineLayer*));

            LinearLayerConfig **hiddenLayerConfigList = getDeepHiddenLayerConfigList(config);
            for (int i=0;i<hiddenLayerCount;++i) {
                LinearLayerConfig *hiddenLayerConfig = hiddenLayerConfigList[i];
                deepNeuralNetwork->hiddenLayerList[i] = buildAffineLayer(hiddenLayerConfig);
            }

            VectorLayer *firstHiddenLayer = (VectorLayer*)deepNeuralNetwork->hiddenLayerList[0];
            setPrevLayer((VectorLayer*)firstHiddenLayer, (VectorLayer*)deepNeuralNetwork->inputLayer);
            setNextLayer((VectorLayer*)deepNeuralNetwork->inputLayer, (VectorLayer*)firstHiddenLayer);

            VectorLayer *lastHiddenLayer = (VectorLayer*)deepNeuralNetwork->hiddenLayerList[hiddenLayerCount - 1];
            setPrevLayer((VectorLayer*)deepNeuralNetwork->outputLayer, (VectorLayer*)lastHiddenLayer);
            setNextLayer((VectorLayer*)lastHiddenLayer, (VectorLayer*)deepNeuralNetwork->outputLayer);

            for (int i=0;i<hiddenLayerCount;++i) {
                if (i < hiddenLayerCount-1) {
                    VectorLayer *prevHiddenLayer = (VectorLayer*)deepNeuralNetwork->hiddenLayerList[i];
                    VectorLayer *nextHiddenLayer = (VectorLayer*)deepNeuralNetwork->hiddenLayerList[i+1];

                    setNextLayer(prevHiddenLayer, nextHiddenLayer);
                    setPrevLayer(nextHiddenLayer, prevHiddenLayer);
                }
            }
        }
    }

    return checkDeepNeuralNetwork(deepNeuralNetwork);
}

bool constructConvNeuralNetwork(ConvNetworkConfig *config) {
    convNeuralNetwork = (ConvNeuralNetwork*)allocate(sizeof(ConvNeuralNetwork));
    if (convNeuralNetwork != NULL) {
        convNeuralNetwork->tensorLayerMap = createHashMap(StringHashCode, StringEqualFun, INITIAL_CONV_LAYER_COUNT);
        
        convNeuralNetwork->trainBatchSize = getConvTrainConfigBatchSize(config);
        convNeuralNetwork->trainEpochCount = getConvTrainConfigEpochCount(config);
        convNeuralNetwork->learnRateValue = getConvLearnRateConfigValue(config);

        int hiddenLayerConfigCount = getConvHiddenLayerConfigCount(config);
        ConvLayerConfig **hiddenLayerConfigList = getConvHiddenLayerConfigList(config);
        Map *tensorLayerMap = convNeuralNetwork->tensorLayerMap;
        for (int i=0;i<hiddenLayerConfigCount;++i) {
            ConvLayerConfig *layerConfig = hiddenLayerConfigList[i];
            String *name = getConvLayerName(layerConfig);
            String *type = getConvLayerType(layerConfig);
            if (strcmp(CONV_LAYER_TYPE, type->getValue(type))==0) {
                ConvLayer *convLayer = buildConvLayer(layerConfig);
                tensorLayerMap->put(tensorLayerMap, name, convLayer);
            } else if (strcmp(POOL_LAYER_TYPE, type->getValue(type))==0) {
                PoolLayer *poolLayer = buildPoolLayer(layerConfig);
                tensorLayerMap->put(tensorLayerMap, name, poolLayer);
            }
        }

        for (int i=0;i<hiddenLayerConfigCount;++i) {
            ConvLayerConfig *layerConfig = hiddenLayerConfigList[i];
            String *name = getConvLayerName(layerConfig);
            String *type = getConvLayerType(layerConfig);

            TensorLayer *tensorLayer = (TensorLayer*)tensorLayerMap->get(tensorLayerMap, name);
            String *nextLayerName = getConvLayerNextName(layerConfig);
            if (nextLayerName != NULL) {
                TensorLayer *nextTensorLayer = tensorLayerMap->get(tensorLayerMap, nextLayerName);
                setNextTensorLayer(tensorLayer, nextTensorLayer);
            }

            String *prevLayerName = getConvLayerPrevName(layerConfig);
            if (prevLayerName != NULL) {
                TensorLayer *prevTensorLayer = tensorLayerMap->get(tensorLayerMap, prevLayerName);
                setNextTensorLayer(tensorLayer, prevTensorLayer);
            }
        }

        ConvLayerConfig *inputLayerConfig = getConvInputLayerConfig(config);
        ConvLayer *inputLayer = buildConvLayer(inputLayerConfig);
        String *nextName = getConvLayerNextName(inputLayerConfig);
        if (tensorLayerMap->containsKey(tensorLayerMap, nextName)) {
            TensorLayer *nextLayer = tensorLayerMap->get(tensorLayerMap, nextName);
            setNextTensorLayer((TensorLayer*)inputLayer, nextLayer);
        }
        convNeuralNetwork->inputLayer = inputLayer;

        ConvOutputLayerConfig *outputLayerConfig = getConvOutputLayerConfig(config);
        ConvOutputLayer *outputLayer = buildConvOutputLayer(outputLayerConfig);
        String *prevName = getConvOutputLayerPrevName(outputLayerConfig);
        if (tensorLayerMap->containsKey(tensorLayerMap, prevName)) {
            TensorLayer *prevLayer = tensorLayerMap->get(tensorLayerMap, prevName);
            setPrevTensorLayer((TensorLayer*)outputLayer, prevLayer);
        }
        convNeuralNetwork->outputLayer = outputLayer;
    }

    return checkConvNeuralNetwork(convNeuralNetwork);
}

bool train(DeepNeuralNetwork *this, TrainBatch *trainBatch, int epoch) {
    int trainDataCount = getTrainDataCount(trainBatch);
    for (int i=0;i<trainDataCount;++i) {
        TrainData *trainData = getTrainData(trainBatch, i);
        predict(this, getDataForTrain(trainData));

        OutputLayer *outputLayer = this->outputLayer;
        float lossValue = loss(outputLayer, getLabelForTrain(trainData));
        logger.info("network train with loss value:%.2f, train batch:%i, epoch:%i", lossValue, i, epoch);

        VectorLayer *baseLayer = (VectorLayer*)outputLayer;
        backward(baseLayer, getLabelForTrain(trainData));
        optimize(baseLayer, this->learnRateValue);
    }
    return true;
}

bool validate(DeepNeuralNetwork *this, TrainBatch *validateBatch) {
    int validateDataCount = getTrainDataCount(validateBatch);
    float percentage = 0.0;
    int successCount = 0;
    for (int i=0;i<validateDataCount;++i) {
        TrainData *validateData = getTrainData(validateBatch, i);
        Vector *predictVector = predict(this, getDataForTrain(validateData));
        Vector *labelVector = getLabelForTrain(validateData);

        int predictValue = getLabelValueFromOneHot(predictVector);
        int labelValue = getLabelValueFromOneHot(labelVector);
        if (predictValue == labelValue) {
            successCount++;
            logger.info("network validation predict with success count:%d, total count:%d", successCount, validateDataCount);
        }
    }

    percentage = (successCount * 100.0f / validateDataCount);
    logger.info("network validation predict with final success rate:%.2f%%", percentage);
    return true;
}

Vector* predict(DeepNeuralNetwork *this, Vector *vector) {
    InputLayer *inputLayer = this->inputLayer;
    OutputLayer *outputLayer = this->outputLayer;
    input(inputLayer, vector);
    return output(outputLayer);
}

int getTrainBatchSize(DeepNeuralNetwork *this) {
    if (this != NULL) {
        return this->trainBatchSize;
    }
    return 0;
}

int getTrainEpochCount(DeepNeuralNetwork *this) {
    if (this != NULL) {
        return this->trainEpochCount;
    }
    return 0;
}

static bool checkDeepNeuralNetwork(DeepNeuralNetwork *neuralNetwork) {
    if (neuralNetwork == NULL) {
        logger.error("neural network instance could not be constructed successfully^o^");
        return false;
    }

    if (neuralNetwork->inputLayer == NULL) {
        logger.error("neural network input layer instance could not be constructed successfully^o^");
        return false;
    }

    if (neuralNetwork->outputLayer == NULL) {
        logger.error("neural network output layer instance could not be constructed successfully^o^");
        return false;
    }

    if (neuralNetwork->hiddenLayerList == NULL) {
        logger.error("neural network hidden layer list could not be constructed successfully^o^");
        return false;
    }

    if (neuralNetwork->hiddenLayerCount == 0) {
        logger.error("neural network hidden layer list is not configured correctly^o^");
        return false;
    }

    for (int i=0;i<neuralNetwork->hiddenLayerCount;++i) {
        AffineLayer *hiddenLayer = neuralNetwork->hiddenLayerList[i];
        if (hiddenLayer == NULL) {
            logger.error("neural network hidden layer instance could not be constructed successfully^o^");
            return false;
        }
    }
    return true;
}

static bool checkConvNeuralNetwork(ConvNeuralNetwork *neuralNetwork) {
    return true;
}

void releaseDeepNeuralNetwork(DeepNeuralNetwork *network) {
    if (network != NULL) {
        releaseInputLayer(network->inputLayer);
        releaseOutputLayer(network->outputLayer);

        for (int i=0;i<network->hiddenLayerCount;++i) {
            releaseAffineLayer(network->hiddenLayerList[i]);
        }
        release(network);
    }
}

void releaseConvNeuralNetwork(ConvNeuralNetwork *network) {
    
}

DeepNeuralNetwork* getDeepNeuralNetwork() {
    return deepNeuralNetwork;
}

ConvNeuralNetwork* getConvNeuralNetwork() {
    return convNeuralNetwork;
}

static int getLabelValueFromOneHot(Vector *vector) {
    if (vector != 0) {
        int count = getElementCount(vector);
        int index = -1;
        float result = 0.0;
        for (int i=0;i<count;++i) {
            float value = getVectorValue(vector, i);
            if (result < value) {
                 result = value;
                 index = i;
            }
        }
        return index;
    }
    return -1;
}