typedef struct Vector Vector;

typedef struct Tensor Tensor;

typedef struct DeepNetworkConfig DeepNetworkConfig;

typedef struct ConvNetworkConfig ConvNetworkConfig;

typedef struct DeepNeuralNetwork DeepNeuralNetwork;

typedef struct ConvNeuralNetwork ConvNeuralNetwork;

bool constructDeepNeuralNetwork(DeepNetworkConfig *config);

bool constructConvNeuralNetwork(ConvNetworkConfig *config);

void releaseDeepNeuralNetwork(DeepNeuralNetwork *network);

void releaseConvNeuralNetwork(ConvNeuralNetwork *network);

bool train(DeepNeuralNetwork *this, TrainBatch *trainBatch, int epoch);

bool validate(DeepNeuralNetwork *this, TrainBatch *trainBatch);

Vector* predict(DeepNeuralNetwork *this, Vector *vector);

int getTrainEpochCount(DeepNeuralNetwork *this);

int getTrainBatchSize(DeepNeuralNetwork *this);

DeepNeuralNetwork* getDeepNeuralNetwork();

ConvNeuralNetwork* getConvNeuralNetwork();