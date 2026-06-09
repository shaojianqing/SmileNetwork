typedef struct Vector Vector;

typedef struct TrainData TrainData;

typedef struct LayerConfig LayerConfig;

typedef struct InputLayer InputLayer;

typedef struct OutputLayer OutputLayer;

typedef struct HiddenLayer HiddenLayer;

typedef struct NetworkConfig NetworkConfig;

typedef struct NeuralNetwork NeuralNetwork;

struct NetworkConfig {

    int trainBatchSize;

    int trainEpochCount;

    float learnRateValue;

    int hiddenLayerConfigCount;

    LayerConfig inputLayerConfig;

    LayerConfig outputLayerConfig;

    LayerConfig *hiddenLayerConfigList;
};

struct NeuralNetwork {

    int trainEpochCount;

    int trainBatchSize;

    float learnRateValue;

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    HiddenLayer **hiddenLayerList;

    int hiddenLayerCount;

    Result* (*train)(NeuralNetwork *this, TrainBatch *trainBatch, int epoch);

    Result* (*predict)(NeuralNetwork *this, Vector *vector);
};

bool constructNeuralNetwork(NetworkConfig *config);

void releaseNeuralNetwork(NeuralNetwork *network);

NeuralNetwork* getNeuralNetwork();
