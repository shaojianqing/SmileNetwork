typedef struct Vector Vector;

typedef struct TrainData TrainData;

typedef struct TrainBatch TrainBatch;

struct TrainData {

    Vector *data;

    Vector *label;
};

struct TrainBatch {

    int dataCount;

    TrainData *dataList;
};

Result* loadTrainBatchStochastic(int batchSize);

void releaseTrainBatch(TrainBatch *trainBatch);