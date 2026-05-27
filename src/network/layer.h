
typedef struct LayerConfig LayerConfig;

typedef struct BaseLayer BaseLayer;

typedef struct InputLayer InputLayer;

typedef struct OutputLayer OutputLayer;

typedef struct HiddenLayer HiddenLayer;

struct LayerConfig {

    bool isOutputLayer;

    int matrixRowCount;

    int matrixColumnCount;

    int biasDimensionCount;

    ActivatorKind activatorKind;

    ActivatorLossKind activatorLossKind;
};

struct BaseLayer {

    Bias *modelBias;

    Matrix *modelMatrix;

    Activator *activator;

    Bias *gradientBias;

    Matrix *gradientMatrix;

    BaseLayer *prevLayer;

    BaseLayer *nextLayer;

    Vector *inputVector;

    Vector *resultVector;

    Result* (*foreward)(BaseLayer *this, Vector *vector);

    Result* (*backward)(BaseLayer *this, Vector *target);

    Result* (*optimize)(BaseLayer *this, float learnRate);
};

struct InputLayer {

    BaseLayer baseLayer;

    Result* (*input)(InputLayer *this, Vector *vector);
};

struct OutputLayer {

    BaseLayer baseLayer;

    ActivatorLossFunc activatorLossFunc;

    ActivatorGradientFunc activatorGradientFunc;

    Result* (*loss)(OutputLayer *this, Vector *expect);

    Result* (*output)(OutputLayer *this);
};

struct HiddenLayer {

    BaseLayer baseLayer;
};

InputLayer *buildInputLayer(LayerConfig *config);

OutputLayer *buildOutputLayer(LayerConfig *config);

HiddenLayer *buildHiddenLayer(LayerConfig *config);

void releaseInputLayer(InputLayer *inputLayer);

void releaseOutputLayer(OutputLayer *outputLayer);

void releaseHiddenLayer(HiddenLayer *hiddenLayer);