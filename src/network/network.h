typedef struct Vector Vector;

typedef struct TrainData TrainData;

typedef struct LayerConfig LayerConfig;

typedef struct InputLayer InputLayer;

typedef struct OutputLayer OutputLayer;

typedef struct HiddenLayer HiddenLayer;

typedef struct NetworkConfig NetworkConfig;

typedef struct NeuralNetwork NeuralNetwork;

struct NetworkConfig {

    LayerConfig inputLayerConfig;

    LayerConfig outputLayerConfig;

    LayerConfig *hiddenLayerConfigList;

    int hiddenLayerConfigCount;

    float learnRateConfigValue;
};

struct NeuralNetwork {

    float learnRateValue;

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    HiddenLayer **hiddenLayerList;

    int hiddenLayerCount;

    Result* (*train)(NeuralNetwork *this, TrainData **tainDataList, int trainDataCount);

    Result* (*predict)(NeuralNetwork *this, Vector *vector);
};

bool constructNeuralNetwork(NetworkConfig *config);

void releaseNeuralNetwork(NeuralNetwork *network);

NeuralNetwork* getNeuralNetwork();
