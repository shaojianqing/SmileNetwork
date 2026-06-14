typedef struct MnistData MnistData;

typedef struct MnistLabel MnistLabel;

bool loadMnistDataFromFile(const char *filename);

bool loadMnistLabelFromFile(const char *filename);

MnistData* getMnistTrainData();

MnistLabel* getMnistTrainLabel();

void releaseMnistData(MnistData *mnistData);

void releaseMnistLabel(MnistLabel *mnistLabel);

int getImageCount(MnistData *mnistData);

byte* getDataBuffer(MnistData *mnistData);

int getMnistRowCount(MnistData *mnistData);

int getMnistColumnCount(MnistData *mnistData);

int getLableCount(MnistLabel *mnistLabel);

int getLabel(MnistLabel *mnistLabel, int index);