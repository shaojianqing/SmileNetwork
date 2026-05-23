
typedef struct LayerConfig LayerConfig;

typedef struct BaseLayer BaseLayer;

typedef struct InputLayer InputLayer;

typedef struct OutputLayer OutputLayer;

typedef struct HiddenLayer HiddenLayer;

struct LayerConfig {

    int matrixRowCount;

    int matrixColumnCount;

    int biasDimensionCount;

    ActivatorKind activatorKind;
};

struct BaseLayer {

    Bias *bias;

    Matrix *matrix;

    Activator *activator;
};

struct InputLayer {

    BaseLayer baseLayer;

    HiddenLayer *nextLayer;

    Result* (*input)(InputLayer *this, Vector *vector);
    
    Result* (*backward)(InputLayer *this, Vector *vector);
};

struct OutputLayer {

    BaseLayer baseLayer;

    HiddenLayer *prevLayer;

    Result* (*output)(OutputLayer *this);

    Result* (*foreward)(OutputLayer *this, Vector *vector);
};

struct HiddenLayer {

    BaseLayer baseLayer;

    InputLayer *inputLayer;

    OutputLayer *outputLayer;

    HiddenLayer *prevLayer;

    HiddenLayer *nextLayer;

    Result* (*foreward)(BaseLayer *this, Vector *vector);

    Result* (*backward)(BaseLayer *this, Vector *vector);
};

InputLayer *buildInputLayer(LayerConfig *config);

OutputLayer *buildOutputLayer(LayerConfig *config);

HiddenLayer *buildHiddenLayer(LayerConfig *config);

void releaseInputLayer(InputLayer *inputLayer);

void releaseOutputLayer(OutputLayer *outputLayer);

void releaseHiddenLayer(HiddenLayer *hiddenLayer);