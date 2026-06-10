typedef struct MnistData MnistData;

typedef struct MnistLabel MnistLabel;

bool loadMnistDataFromFile(const char *filename);

bool loadMnistLabelFromFile(const char *filename);

MnistData* getMnistTrainData();

MnistLabel* getMnistTrainLabel();

int getImageCount(MnistData *mnistData);

byte* getDataBuffer(MnistData *mnistData);

int getMnistRowCount(MnistData *mnistData);

int getMnistColumnCount(MnistData *mnistData);

int getLableCount(MnistLabel *mnistLabel);

int getLabel(MnistLabel *mnistLabel, int index);