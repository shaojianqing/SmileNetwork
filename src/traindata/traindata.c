#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../random/random.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"
#include "../network/bias.h"
#include "../network/vector.h"
#include "../network/label.h"

#include "traindata.h"

static void retain(TrainData *this);

static void release(TrainData *this);

TrainData *createTrainData(Vector *data, Label *label) {
    TrainData *trainData = (TrainData*)malloc(sizeof(TrainData));
    if (trainData != NULL) {
        trainData->data = data;
        trainData->label = label;

        trainData->retain = retain;
        trainData->release = release;
    }
    return trainData;
}

static void retain(TrainData *this) {
    
}

static void release(TrainData *this) {

}