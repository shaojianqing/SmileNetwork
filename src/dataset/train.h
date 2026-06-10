typedef struct Vector Vector;

typedef struct TrainData TrainData;

typedef struct TrainBatch TrainBatch;

Result* loadTrainBatchStochastic(int batchSize);

void releaseTrainBatch(TrainBatch *trainBatch);

int getTrainDataCount(TrainBatch *this);

TrainData* getTrainData(TrainBatch *this, int index);

Vector* getDataFroTrain(TrainData* this);

Vector* getLabelFroTrain(TrainData* this);