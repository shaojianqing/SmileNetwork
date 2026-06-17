#include <stdio.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../logger/logger.h"
#include "../memory/memory.h"
#include "../result/result.h"
#include "../random/random.h"
#include "../dataset/train.h"

#include "config.h"
#include "activator.h"
#include "bias.h"
#include "loss.h"
#include "vector.h"
#include "matrix.h"
#include "layer.h"
#include "network.h"

struct NeuralNetwork {

    int trainEpochCount;

    int trainBatchSize;

    float learnRateValue;

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    LinearLayer **hiddenLayerList;

    int hiddenLayerCount;
};

extern Logger logger;

static NeuralNetwork *neuralNetwork;

static int getLabelValueFromOneHot(Vector *vector);

static bool checkNeuralNetwork(NeuralNetwork *neuralNetwork);

bool constructNeuralNetwork(NetworkConfig *config) {
    neuralNetwork = (NeuralNetwork*)allocate(sizeof(NeuralNetwork));
    if (neuralNetwork != NULL) {
        neuralNetwork->trainBatchSize = getTrainConfigBatchSize(config);
        neuralNetwork->trainEpochCount = getTrainConfigEpochCount(config);
        neuralNetwork->learnRateValue = getLearnRateConfigValue(config);

        LayerConfig *inputLayerConfig = getInputLayerConfig(config);
        neuralNetwork->inputLayer = buildInputLayer(inputLayerConfig);

        LayerConfig *outputLayerConfig = getOutputLayerConfig(config);
        neuralNetwork->outputLayer = buildOutputLayer(outputLayerConfig);

        int hiddenLayerCount = getHiddenLayerConfigCount(config);
        if (hiddenLayerCount > 0) {
            neuralNetwork->hiddenLayerCount = hiddenLayerCount;
            neuralNetwork->hiddenLayerList = (LinearLayer **)allocate(hiddenLayerCount * sizeof(LinearLayer*));

            LayerConfig **hiddenLayerConfigList = getHiddenLayerConfigList(config);
            for (int i=0;i<hiddenLayerCount;++i) {
                LayerConfig *hiddenLayerConfig = hiddenLayerConfigList[i];
                neuralNetwork->hiddenLayerList[i] = buildLinearLayer(hiddenLayerConfig);
            }

            BaseLayer *firstHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[0];
            setPrevLayer((BaseLayer*)firstHiddenLayer, (BaseLayer*)neuralNetwork->inputLayer);
            setNextLayer((BaseLayer*)neuralNetwork->inputLayer, (BaseLayer*)firstHiddenLayer);

            BaseLayer *lastHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[hiddenLayerCount - 1];
            setPrevLayer((BaseLayer*)neuralNetwork->outputLayer, (BaseLayer*)lastHiddenLayer);
            setNextLayer((BaseLayer*)lastHiddenLayer, (BaseLayer*)neuralNetwork->outputLayer);

            for (int i=0;i<hiddenLayerCount;++i) {
                if (i < hiddenLayerCount-1) {
                    BaseLayer *prevHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[i];
                    BaseLayer *nextHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[i+1];

                    setNextLayer(prevHiddenLayer, nextHiddenLayer);
                    setPrevLayer(nextHiddenLayer, prevHiddenLayer);
                }
            }
        }
    }
    return checkNeuralNetwork(neuralNetwork);
}

Result* train(NeuralNetwork *this, TrainBatch *trainBatch, int epoch) {
    int trainDataCount = getTrainDataCount(trainBatch);
    for (int i=0;i<trainDataCount;++i) {
        TrainData *trainData = getTrainData(trainBatch, i);
        Result *predictResult = predict(this, getDataForTrain(trainData));
        if (!success(predictResult)) {
            logger.error("network train predict phase with error[code:%d, message:%s]", getCode(predictResult), getMessage(predictResult));
            return predictResult;
        }
        releaseResult(predictResult);

        OutputLayer *outputLayer = this->outputLayer;
        Result *lossResult = loss(outputLayer, getLabelForTrain(trainData));
        if (!success(lossResult)) {
            logger.error("network train with loss error[code:%d, message:%s]", getCode(lossResult), getMessage(lossResult));
            return lossResult;
        } else {
            float lossValue = getValue(lossResult);
            releaseResult(lossResult);
            logger.info("network train with loss value:%.2f, train batch:%i, epoch:%i", lossValue, i, epoch);
        }

        BaseLayer *baseLayer = (BaseLayer*)outputLayer;
        Result *backwardResult =  backward(baseLayer, getLabelForTrain(trainData));
        if (!success(backwardResult)) {
            logger.error("network train backward phase with error[code:%d, message:%s]", getCode(backwardResult), getMessage(backwardResult));
            return backwardResult;
        }
        releaseResult(backwardResult);

        Result *optimizeResult = optimize(baseLayer, this->learnRateValue);
        if (!success(optimizeResult)) {
            logger.error("network train optimize phase with error[code:%d, message:%s]", getCode(optimizeResult), getMessage(optimizeResult));
            return optimizeResult;
        }
        releaseResult(optimizeResult);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* validate(NeuralNetwork *this, TrainBatch *validateBatch) {
    int validateDataCount = getTrainDataCount(validateBatch);
    float percentage = 0.0;
    int successCount = 0;
    for (int i=0;i<validateDataCount;++i) {
        TrainData *validateData = getTrainData(validateBatch, i);
        Result *predictResult = predict(this, getDataForTrain(validateData));
        if (!success(predictResult)) {
            logger.error("network validation predict phase with error[code:%d, message:%s]", getCode(predictResult), getMessage(predictResult));
            return predictResult;
        }
    
        Vector *predictVector = (Vector *)getData(predictResult);
        Vector *labelVector = getLabelForTrain(validateData);
        releaseResult(predictResult);

        int predictValue = getLabelValueFromOneHot(predictVector);
        int labelValue = getLabelValueFromOneHot(labelVector);
        if (predictValue == labelValue) {
            successCount++;
            logger.info("network validation predict with success count:%d, total count:%d", successCount, validateDataCount);
        }
    }

    percentage = (successCount * 100.0f / validateDataCount);
    logger.info("network validation predict with final success rate:%.2f%%", percentage);
    return createResultWithoutData(SUCCESS, NULL);
}

Result* predict(NeuralNetwork *this, Vector *vector) {
    InputLayer *inputLayer = this->inputLayer;
    OutputLayer *outputLayer = this->outputLayer;
    Result *inputResult = input(inputLayer, vector);
    if (!success(inputResult)) {
        logger.error("network predict with error[code:%d, message:%s]", getCode(inputResult), getMessage(inputResult));
        return inputResult;
    }
    releaseResult(inputResult);
    return output(outputLayer);
}

int getTrainBatchSize(NeuralNetwork *this) {
    if (this != NULL) {
        return this->trainBatchSize;
    }
    return 0;
}

int getTrainEpochCount(NeuralNetwork *this) {
    if (this != NULL) {
        return this->trainEpochCount;
    }
    return 0;
}

NeuralNetwork* getNeuralNetwork() {
    return neuralNetwork;
}

static bool checkNeuralNetwork(NeuralNetwork *neuralNetwork) {
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
        LinearLayer *hiddenLayer = neuralNetwork->hiddenLayerList[i];
        if (hiddenLayer == NULL) {
            logger.error("neural network hidden layer instance could not be constructed successfully^o^");
            return false;
        }
    }
    return true;
}

void releaseNeuralNetwork(NeuralNetwork *network) {
    if (network != NULL) {
        releaseInputLayer(network->inputLayer);
        releaseOutputLayer(network->outputLayer);

        for (int i=0;i<network->hiddenLayerCount;++i) {
            releaseLinearLayer(network->hiddenLayerList[i]);
        }
        release(network);
    }
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