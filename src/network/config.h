typedef struct ConvLayerConfig ConvLayerConfig;

typedef struct PoolLayerConfig PoolLayerConfig;

typedef struct LinearLayerConfig LinearLayerConfig;

typedef struct DeepNetworkConfig DeepNetworkConfig;

typedef struct ConvNetworkConfig ConvNetworkConfig;

typedef enum ActivatorKind ActivatorKind;

typedef enum ActivatorLossKind ActivatorLossKind;

DeepNetworkConfig* loadNetworkConfig(char *filepath);

void releaseDeepNetworkConfig(DeepNetworkConfig *config);

int getTrainConfigBatchSize(DeepNetworkConfig *config);

int getTrainConfigEpochCount(DeepNetworkConfig *config);

float getLearnRateConfigValue(DeepNetworkConfig *config);

int getHiddenLayerConfigCount(DeepNetworkConfig *config);

LinearLayerConfig* getInputLayerConfig(DeepNetworkConfig *config);

LinearLayerConfig* getOutputLayerConfig(DeepNetworkConfig *config);

LinearLayerConfig** getHiddenLayerConfigList(DeepNetworkConfig *config);


bool isOutputLayer(LinearLayerConfig *config);

int getMatrixConfigRowCount(LinearLayerConfig *config);

int getMatrixConfigColumnCount(LinearLayerConfig *config);

int getBiasConfigDimensionCount(LinearLayerConfig *config);

ActivatorKind getConfigActivatorKind(LinearLayerConfig *config);

ActivatorLossKind getConfigActivatorLossKind(LinearLayerConfig *config);

