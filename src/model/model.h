typedef struct Header Header;

typedef struct Item Item;

typedef struct Body Body;

typedef struct Model Model;

Model* loadModelFromFile(const char* filename);

int saveModelToFile(const char* filename, Model* model);