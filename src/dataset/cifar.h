typedef struct CifarData CifarData;

void loadCifarTrainDataFromFile(const char *filename);

void loadCifarValidateDataFromFile(const char *filename);

List* getCifarTrainDataList();

void releaseCifarDataList(List* *cifarDataList);