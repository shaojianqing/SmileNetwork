#include <stdio.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../logger/logger.h"
#include "../memory/memory.h"
#include "../datatype/datatype.h"
#include "../datatype/hashmap.h"
#include "../except/exception.h"
#include "../generator/generator.h"
#include "../dataset/train.h"

#include "config.h"
#include "activator.h"
#include "bias.h"
#include "loss.h"
#include "vector.h"
#include "matrix.h"
#include "linear.h"
#include "network.h"

struct DeepNeuralNetwork {

    int trainEpochCount;

    int trainBatchSize;

    float learnRateValue;

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    AffineLayer **hiddenLayerList;

    int hiddenLayerCount;
};

struct ConvNeuralNetwork {

    int trainEpochCount;

    float learnRateValue;

    Map *tensorLayerMap;

};

extern Logger logger;

static DeepNeuralNetwork *deepNeuralNetwork;

Exception TestException = {"", "ewewe", 0};

static int getLabelValueFromOneHot(Vector *vector);

static bool checkNeuralNetwork(DeepNeuralNetwork *neuralNetwork);

bool constructDeepNeuralNetwork(DeepNetworkConfig *config) {
    DeepNeuralNetwork *neuralNetwork = (DeepNeuralNetwork*)allocate(sizeof(DeepNeuralNetwork));
    if (neuralNetwork != NULL) {
        neuralNetwork->trainBatchSize = getTrainConfigBatchSize(config);
        neuralNetwork->trainEpochCount = getTrainConfigEpochCount(config);
        neuralNetwork->learnRateValue = getLearnRateConfigValue(config);

        LinearLayerConfig *inputLayerConfig = getInputLayerConfig(config);
        neuralNetwork->inputLayer = buildInputLayer(inputLayerConfig);

        LinearLayerConfig *outputLayerConfig = getOutputLayerConfig(config);
        neuralNetwork->outputLayer = buildOutputLayer(outputLayerConfig);

        int hiddenLayerCount = getHiddenLayerConfigCount(config);
        if (hiddenLayerCount > 0) {
            neuralNetwork->hiddenLayerCount = hiddenLayerCount;
            neuralNetwork->hiddenLayerList = (AffineLayer **)allocate(hiddenLayerCount * sizeof(AffineLayer*));

            LinearLayerConfig **hiddenLayerConfigList = getHiddenLayerConfigList(config);
            for (int i=0;i<hiddenLayerCount;++i) {
                LinearLayerConfig *hiddenLayerConfig = hiddenLayerConfigList[i];
                neuralNetwork->hiddenLayerList[i] = buildAffineLayer(hiddenLayerConfig);
            }

            VectorLayer *firstHiddenLayer = (VectorLayer*)neuralNetwork->hiddenLayerList[0];
            setPrevLayer((VectorLayer*)firstHiddenLayer, (VectorLayer*)neuralNetwork->inputLayer);
            setNextLayer((VectorLayer*)neuralNetwork->inputLayer, (VectorLayer*)firstHiddenLayer);

            VectorLayer *lastHiddenLayer = (VectorLayer*)neuralNetwork->hiddenLayerList[hiddenLayerCount - 1];
            setPrevLayer((VectorLayer*)neuralNetwork->outputLayer, (VectorLayer*)lastHiddenLayer);
            setNextLayer((VectorLayer*)lastHiddenLayer, (VectorLayer*)neuralNetwork->outputLayer);

            for (int i=0;i<hiddenLayerCount;++i) {
                if (i < hiddenLayerCount-1) {
                    VectorLayer *prevHiddenLayer = (VectorLayer*)neuralNetwork->hiddenLayerList[i];
                    VectorLayer *nextHiddenLayer = (VectorLayer*)neuralNetwork->hiddenLayerList[i+1];

                    setNextLayer(prevHiddenLayer, nextHiddenLayer);
                    setPrevLayer(nextHiddenLayer, prevHiddenLayer);
                }
            }
        }
    }
    return checkNeuralNetwork(neuralNetwork);
}

bool constructConvNeuralNetwork(ConvNetworkConfig *config) {
    return true;
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

DeepNeuralNetwork* getDeepNeuralNetwork() {
    return deepNeuralNetwork;
}

static bool checkNeuralNetwork(DeepNeuralNetwork *neuralNetwork) {
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