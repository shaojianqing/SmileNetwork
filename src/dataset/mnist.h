typedef struct MnistData MnistData;

typedef struct MnistLabel MnistLabel;

struct MnistData {
    int magic;

    int imageCount;

    int rowCount;

    int columnCount;

    byte *dataBuffer;
};

struct MnistLabel {
    int magic;

    int labelCount;

    byte *labelBuffer;
};

bool loadMnistDataFromFile(const char *filename);

bool loadMnistLabelFromFile(const char *filename);

MnistData* getMnistTrainData();

MnistLabel* getMnistTrainLabel();