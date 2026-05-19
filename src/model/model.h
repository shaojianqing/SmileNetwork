typedef struct Header Header;

typedef struct Item Item;

typedef struct Body Body;

typedef struct Model Model;

struct Header {

    char magic[8];

    int itemCount;

    Item *itemList;
};

struct Item {

};

struct Body {
    byte *data;
};

struct Model {

    Header *header;

    Body *body;
};

Model* loadModeFromFile(const char* filename);

int saveModelToFile(Model* model, const char* filename);