typedef struct Vector Vector;

typedef struct TrainData TrainData;

typedef struct TrainBatch TrainBatch;

TrainBatch* loadTrainBatchStochastic(int batchSize);

TrainBatch* loadTrainBatchForValidate();

void releaseTrainBatch(TrainBatch *trainBatch);

int getTrainDataCount(TrainBatch *this);

TrainData* getTrainData(TrainBatch *this, int index);

Vector* getDataForTrain(TrainData* this);

Vector* getLabelForTrain(TrainData* this);