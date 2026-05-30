#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../result/result.h"
#include "../datatype/stringtype.h"
#include "../network/bias.h"
#include "../network/vector.h"
#include "../network/label.h"

#include "traindata.h"

TrainData *createTrainData(Vector *data, Label *label) {
    TrainData *trainData = (TrainData*)allocate(sizeof(TrainData));
    if (trainData != NULL) {
        trainData->data = data;
        trainData->label = label;

    }
    return trainData;
}