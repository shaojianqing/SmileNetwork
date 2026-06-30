typedef struct ConvLayer ConvLayer;

typedef struct PoolLayer PoolLayer;

ConvLayer* createConvLayer(int fieldSize, int kernelCount, int channelCount);

PoolLayer* createPoolLayer(int poolSize, int poolStride);

void releaseConvLayer(ConvLayer* convLayer);

void releasePoolLayer(PoolLayer *poolLayer);

void inputConv(ConvLayer *this, Tensor *tensor);