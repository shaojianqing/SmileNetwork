typedef struct LayerConfig LayerConfig;

typedef struct NetworkConfig NetworkConfig;

typedef enum ActivatorKind ActivatorKind;

typedef enum ActivatorLossKind ActivatorLossKind;

Result *loadNetworkConfig(char *filepath);

void releaseNetworkConfig(NetworkConfig *config);

int getTrainConfigBatchSize(NetworkConfig *config);

int getTrainConfigEpochCount(NetworkConfig *config);

float getLearnRateConfigValue(NetworkConfig *config);

int getHiddenLayerConfigCount(NetworkConfig *config);

LayerConfig* getInputLayerConfig(NetworkConfig *config);

LayerConfig* getOutputLayerConfig(NetworkConfig *config);

LayerConfig** getHiddenLayerConfigList(NetworkConfig *config);


bool isOutputLayer(LayerConfig *config);

int getMatrixConfigRowCount(LayerConfig *config);

int getMatrixConfigColumnCount(LayerConfig *config);

int getBiasConfigDimensionCount(LayerConfig *config);

ActivatorKind getConfigActivatorKind(LayerConfig *config);

ActivatorLossKind getConfigActivatorLossKind(LayerConfig *config);

