#define CONV_NETWORK_TYPE "ConvNetwork"
#define DEEP_NETWORK_TYPE "DeepNetwork"

#define CONV_LAYER_TYPE     "convolution"
#define POOL_LAYER_TYPE     "pool"
#define AFFINE_LAYER_TYPE   "affine"

typedef struct ConvLayerConfig ConvLayerConfig;

typedef struct ConvOutputLayerConfig ConvOutputLayerConfig;

typedef struct LinearLayerConfig LinearLayerConfig;

typedef struct DeepNetworkConfig DeepNetworkConfig;

typedef struct ConvNetworkConfig ConvNetworkConfig;

typedef enum ActivatorKind ActivatorKind;

typedef enum ActivatorLossKind ActivatorLossKind;

Json* loadJsonConfigData(char *filepath);

char* getNetworkConfigType(Json *configJson);

DeepNetworkConfig* loadDeepNetworkConfig(Json *configJson);

ConvNetworkConfig* loadConvNetworkConfig(Json *configJson);

// this part is specific for deep neural network(multiply-layer perceptron)
void releaseDeepNetworkConfig(DeepNetworkConfig *config);

void releaseConvNetworkConfig(ConvNetworkConfig *config);

int getDeepTrainConfigBatchSize(DeepNetworkConfig *config);

int getDeepTrainConfigEpochCount(DeepNetworkConfig *config);

float getDeepLearnRateConfigValue(DeepNetworkConfig *config);

int getDeepHiddenLayerConfigCount(DeepNetworkConfig *config);

LinearLayerConfig* getDeepInputLayerConfig(DeepNetworkConfig *config);

LinearLayerConfig* getDeepOutputLayerConfig(DeepNetworkConfig *config);

LinearLayerConfig** getDeepHiddenLayerConfigList(DeepNetworkConfig *config);


bool isLinearOutputLayer(LinearLayerConfig *config);

int getLinearMatrixConfigRowCount(LinearLayerConfig *config);

int getLinearMatrixConfigColumnCount(LinearLayerConfig *config);

int getLinearBiasConfigDimensionCount(LinearLayerConfig *config);

ActivatorKind getLinearConfigActivatorKind(LinearLayerConfig *config);

ActivatorLossKind getLinearConfigActivatorLossKind(LinearLayerConfig *config);


// this part is specific for convolution neural network
int getConvTrainConfigBatchSize(ConvNetworkConfig *config);

int getConvTrainConfigEpochCount(ConvNetworkConfig *config);

float getConvLearnRateConfigValue(ConvNetworkConfig *config);

int getConvHiddenLayerConfigCount(ConvNetworkConfig *config);

ConvLayerConfig* getConvInputLayerConfig(ConvNetworkConfig *config);

ConvOutputLayerConfig* getConvOutputLayerConfig(ConvNetworkConfig *config);

ConvLayerConfig** getConvHiddenLayerConfigList(ConvNetworkConfig *config);

String* getConvLayerName(ConvLayerConfig* config);

String* getConvLayerType(ConvLayerConfig* config);

int getConvLayerFieldSize(ConvLayerConfig* config);

int getConvLayerKernelCount(ConvLayerConfig* config);

int getConvLayerChannelCount(ConvLayerConfig* config);

int getConvLayerPaddingSize(ConvLayerConfig* config);

int getConvLayerPoolSize(ConvLayerConfig* config);

int getConvLayerPoolStride(ConvLayerConfig* config);

String* getConvLayerPrevName(ConvLayerConfig* config);

String* getConvLayerNextName(ConvLayerConfig* config);

String* getConvOutputLayerPrevName(ConvOutputLayerConfig* config);

int getConvOutputMatrixConfigRowCount(ConvOutputLayerConfig *config);

int getConvOutputMatrixConfigColumnCount(ConvOutputLayerConfig *config);

int getConvOutputBiasConfigDimensionCount(ConvOutputLayerConfig *config);

ActivatorKind getConvOutputConfigActivatorKind(ConvOutputLayerConfig *config);

ActivatorLossKind getConvOutputConfigActivatorLossKind(ConvOutputLayerConfig *config);