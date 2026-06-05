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

extern Logger logger;

static NeuralNetwork *neuralNetwork;

static bool checkNeuralNetwork(NeuralNetwork *neuralNetwork);

static Result* train(NeuralNetwork *this, TrainBatch *trainBatch);

static Result* predict(NeuralNetwork *this, Vector *vector);

bool constructNeuralNetwork(NetworkConfig *config) {
    neuralNetwork = (NeuralNetwork*)allocate(sizeof(NeuralNetwork));
    if (neuralNetwork != NULL) {
        neuralNetwork->train = train;
        neuralNetwork->predict = predict;

        neuralNetwork->trainBatchSize = config->trainBatchSize;
        neuralNetwork->trainEpochCount = config->trainEpochCount;
        neuralNetwork->learnRateValue = config->learnRateValue;

        neuralNetwork->inputLayer = buildInputLayer(config->inputLayerConfig);
        neuralNetwork->outputLayer = buildOutputLayer(config->outputLayerConfig);

        if (config->hiddenLayerConfigCount > 0) {
            int hiddenLayerCount = config->hiddenLayerConfigCount;
            neuralNetwork->hiddenLayerCount = hiddenLayerCount;
            neuralNetwork->hiddenLayerList = (HiddenLayer **)allocate(hiddenLayerCount * sizeof(HiddenLayer*));

            int i = 0;
            for (i = 0;i<hiddenLayerCount;++i) {
                LayerConfig hiddenLayerConfig = config->hiddenLayerConfigList[i];
                neuralNetwork->hiddenLayerList[i] = buildHiddenLayer(hiddenLayerConfig);
            }

            BaseLayer *firstHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[0];
            firstHiddenLayer->prevLayer = (BaseLayer*)neuralNetwork->inputLayer;
            ((BaseLayer*)neuralNetwork->inputLayer)->nextLayer = firstHiddenLayer;

            BaseLayer *lastHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[hiddenLayerCount - 1];
            lastHiddenLayer->nextLayer = (BaseLayer*)neuralNetwork->outputLayer;
            ((BaseLayer*)neuralNetwork->outputLayer)->prevLayer = lastHiddenLayer;

            for (i = 0;i<hiddenLayerCount;++i) {
                if (i < hiddenLayerCount-1) {
                    BaseLayer *prevHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[i];
                    BaseLayer *nextHiddenLayer = (BaseLayer*)neuralNetwork->hiddenLayerList[i+1];

                    prevHiddenLayer->nextLayer = nextHiddenLayer;
                    nextHiddenLayer->prevLayer = prevHiddenLayer;
                }
            }
        }
    }
    return checkNeuralNetwork(neuralNetwork);
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

    int i = 0;
    for (i=0;i<neuralNetwork->hiddenLayerCount;++i) {
        HiddenLayer *hiddenLayer = neuralNetwork->hiddenLayerList[i];
        if (hiddenLayer == NULL) {
            logger.error("neural network hidden layer instance could not be constructed successfully^o^");
            return false;
        }
    }
    return true;
}

NeuralNetwork* getNeuralNetwork() {
    return neuralNetwork;
}

void releaseNeuralNetwork(NeuralNetwork *network) {
    if (network != NULL) {
        releaseInputLayer(network->inputLayer);
        releaseOutputLayer(network->outputLayer);
        int i = 0;
        for (i=0;i<network->hiddenLayerCount;++i) {
            releaseHiddenLayer(network->hiddenLayerList[i]);
        }

        release(network);
    }
}

static Result* train(NeuralNetwork *this, TrainBatch *trainBatch) {
    int i = 0;
    for (i=0;i<trainBatch->dataCount;++i) {
        TrainData trainData = trainBatch->dataList[i];
        Result *predictResult = predict(this, trainData.data);
        if (!predictResult->success(predictResult)) {
            logger.error("network train with error[code:%d, message:%s]", predictResult->code, predictResult->message);
            return predictResult;
        }
        releaseResult(predictResult);

        OutputLayer *outputLayer = this->outputLayer;

        Result *lossResult = outputLayer->loss(outputLayer, trainData.label);
        if (lossResult->success(lossResult)) {
            float lossValue = lossResult->getValue(lossResult);
            logger.info("network train with loss value:%.2f, train batch:%i", lossValue, i);

            BaseLayer *baseLayer = (BaseLayer*)outputLayer;
            baseLayer->backward(baseLayer, trainData.label);
            baseLayer->optimize(baseLayer, this->learnRateValue);
        } else {
            logger.error("network train with loss error[code:%d, message:%s]", lossResult->code, lossResult->message);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* predict(NeuralNetwork *this, Vector *vector) {
    InputLayer *inputLayer = this->inputLayer;
    OutputLayer *outputLayer = this->outputLayer;
    Result *inputResult = inputLayer->input(inputLayer, vector);
    if (!inputResult->success(inputResult)) {
        logger.error("network predict with error[code:%d, message:%s]", inputResult->code, inputResult->message);
        return inputResult;
    }
    return outputLayer->output(outputLayer);
}