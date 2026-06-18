typedef struct Vector Vector;

typedef struct NetworkConfig NetworkConfig;

typedef struct NeuralNetwork NeuralNetwork;

bool constructNeuralNetwork(NetworkConfig *config);

void releaseNeuralNetwork(NeuralNetwork *network);

bool train(NeuralNetwork *this, TrainBatch *trainBatch, int epoch);

bool validate(NeuralNetwork *this, TrainBatch *trainBatch);

Vector* predict(NeuralNetwork *this, Vector *vector);

int getTrainEpochCount(NeuralNetwork *this);

int getTrainBatchSize(NeuralNetwork *this);

NeuralNetwork* getNeuralNetwork();