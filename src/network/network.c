#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"

#include "result.h"
#include "layer.h"
#include "network.h"

static void addLayer(Network *this, Layer *layer);

static Result* train(Network *this, TrainData **tainDataList, int trainDataCount);

static Result* predict(Network *this, Vector *vector);

Network* createNetwork() {
    Network *network = (Network*)malloc(sizeof(Network));
    if (network != NULL) {

        network->train = train;
        network->predict = predict;
        network->addLayer = addLayer;
        
        network->retain = retain;
        network->release = release;
    }
    return vector;
}

static void addLayer(Network *this, Layer *layer) {

}

static Result* train(Network *this, TrainData **tainDataList, int trainDataCount) {

}

static Result* predict(Network *this, Vector *vector) {

}