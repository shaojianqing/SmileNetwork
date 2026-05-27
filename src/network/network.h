typedef struct NetworkConfig NetworkConfig;

typedef struct NeuralNetwork NeuralNetwork;

struct NetworkConfig {

    LayerConfig *inputLayerConfig;

    LayerConfig *outputLayerConfig;

    LayerConfig **hiddenLayerConfigList;

    int hiddenLayerConfigCount;

    float learnRateConfigValue;
};

struct NeuralNetwork {

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    HiddenLayer **hiddenLayerList;

    int hiddenLayerCount;

    float learnRateValue;

    Result* (*train)(NeuralNetwork *this, TrainData **tainDataList, int trainDataCount);

    Result* (*predict)(NeuralNetwork *this, Vector *vector);
};

NeuralNetwork* buildNeuralNetwork(NetworkConfig *config);

NeuralNetwork* getNeuralNetwork();

