
typedef struct ConvLayer ConvLayer;

typedef struct PoolLayer PoolLayer;

typedef struct TensorLayer TensorLayer;

typedef struct ConvOutputLayer ConvOutputLayer;

ConvLayer* buildConvLayer(ConvLayerConfig *config);

PoolLayer* buildPoolLayer(ConvLayerConfig *config);

ConvOutputLayer* buildConvOutputLayer(ConvOutputLayerConfig *config);

void releaseConvLayer(ConvLayer* convLayer);

void releasePoolLayer(PoolLayer *poolLayer);

void releaseConvOutputLayer(ConvOutputLayer *outputLayer);

void inputConv(ConvLayer *this, Tensor *tensor);

void setNextTensorLayer(TensorLayer *this, TensorLayer *nextLayer);

void setPrevTensorLayer(TensorLayer *this, TensorLayer *prevLayer);