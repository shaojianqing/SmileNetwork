#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"

#include "model.h"

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

Model* loadModelFromFile(const char* filename) {
    return NULL;
}

int saveModelToFile(const char* filename, Model* model) {
    return 0;
}