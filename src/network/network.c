#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../logger/logger.h"
#include "../traindata/traindata.h"

#include "result.h"
#include "vector.h"
#include "layer.h"
#include "network.h"

extern Logger logger;

static NeuralNetwork *neuralNetwork;

static Result* train(NeuralNetwork *this, TrainData **tainDataList, int trainDataCount);

static Result* predict(NeuralNetwork *this, Vector *vector);

NeuralNetwork* buildNeuralNetwork(NetworkConfig *config) {
    NeuralNetwork *network = (NeuralNetwork*)malloc(sizeof(NeuralNetwork));
    if (network != NULL) {
        network->train = train;
        network->predict = predict;

        network->inputLayer = buildInputLayer(config->inputLayerConfig);
        network->outputLayer = buildOutputLayer(config->outputLayerConfig);

        if (hiddenLayerCount > 0) {
            int hiddenLayerCount = config->hiddenLayerConfigCount;
            network->hiddenLayerList = (HiddenLayer **)malloc(hiddenLayerCount * sizeof(HiddenLayer *));

            int i = 0;
            for (i = 0;i<hiddenLayerCount;++i) {
                LayerConfig *hiddenLayerConfig = config->hiddenLayerConfigList[i];
                network->hiddenLayerList[i] = buildHiddenLayer(hiddenLayerConfig);
            }

            HiddenLayer *firstHiddenLayer = network->hiddenLayerList[0];
            firstHiddenLayer->inputLayer = network->inputLayer;

            HiddenLayer *lastHiddenLayer = network->hiddenLayerList[hiddenLayerCount - 1];
            lastHiddenLayer->outputLayer = network->outputLayer;

            for (i = 0;i<hiddenLayerCount;++i) {
                if (i < hiddenLayerCount-1) {
                    HiddenLayer *prevHiddenLayer = network->hiddenLayerList[i];
                    HiddenLayer *nextHiddenLayer = network->hiddenLayerList[i+1];

                    prevHiddenLayer->nextLayer = nextHiddenLayer;
                    nextHiddenLayer->prevLayer = prevHiddenLayer;
                }
            }
        }
    }
    return network;
}

NeuralNetwork* getNeuralNetwork() {
    return neuralNetwork;
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
    } else {
        logger.error("network predict with error[code:%d, message:%s]", inputResult->code, inputResult->message);
    }
    releaseResult(inputResult);
}