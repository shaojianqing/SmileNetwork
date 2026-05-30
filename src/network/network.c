#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../logger/logger.h"
#include "../memory/memory.h"
#include "../result/result.h"
#include "../random/random.h"
#include "../traindata/traindata.h"

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

static Result* train(NeuralNetwork *this, TrainData **tainDataList, int trainDataCount);

static Result* predict(NeuralNetwork *this, Vector *vector);

bool constructNeuralNetwork(NetworkConfig *config) {
    NeuralNetwork *network = (NeuralNetwork*)allocate(sizeof(NeuralNetwork));
    if (network != NULL) {
        network->train = train;
        network->predict = predict;

        network->inputLayer = buildInputLayer(config->inputLayerConfig);
        network->outputLayer = buildOutputLayer(config->outputLayerConfig);

        if (config->hiddenLayerConfigCount > 0) {
            int hiddenLayerCount = config->hiddenLayerConfigCount;
            network->hiddenLayerList = (HiddenLayer **)allocate(hiddenLayerCount * sizeof(HiddenLayer*));

            int i = 0;
            for (i = 0;i<hiddenLayerCount;++i) {
                LayerConfig hiddenLayerConfig = config->hiddenLayerConfigList[i];
                network->hiddenLayerList[i] = buildHiddenLayer(hiddenLayerConfig);
            }

            BaseLayer *firstHiddenLayer = (BaseLayer*)network->hiddenLayerList[0];
            firstHiddenLayer->prevLayer = (BaseLayer*)network->inputLayer;

            BaseLayer *lastHiddenLayer = (BaseLayer*)network->hiddenLayerList[hiddenLayerCount - 1];
            lastHiddenLayer->nextLayer = (BaseLayer*)network->outputLayer;

            for (i = 0;i<hiddenLayerCount;++i) {
                if (i < hiddenLayerCount-1) {
                    BaseLayer *prevHiddenLayer = (BaseLayer*)network->hiddenLayerList[i];
                    BaseLayer *nextHiddenLayer = (BaseLayer*)network->hiddenLayerList[i+1];

                    prevHiddenLayer->nextLayer = nextHiddenLayer;
                    nextHiddenLayer->prevLayer = prevHiddenLayer;
                }
            }
        }
    }
    neuralNetwork = network;
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

}

static Result* train(NeuralNetwork *this, TrainData **tainDataList, int trainDataCount) {
    return NULL;
}

static Result* predict(NeuralNetwork *this, Vector *vector) {
    InputLayer *inputLayer = this->inputLayer;
    OutputLayer *outputLayer = this->outputLayer;
    Result* inputResult = inputLayer->input(inputLayer, vector);
    if (inputResult->success(inputResult)) {
        return outputLayer->output(outputLayer);
    }

    logger.error("network predict with error[code:%d, message:%s]", inputResult->code, inputResult->message);
    return inputResult;
}