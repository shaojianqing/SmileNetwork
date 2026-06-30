
#include "../datatype/arraylist.h"

#include "cifar.h"

struct CifarData {

    
};

static List *cifarTrainDataList;

static List *cifarValidateDataList;

static CifarData* loadCifarDataFromFile(const char *filename);

void loadCifarTrainDataFromFile(const char *filename) {

}

void loadCifarValidateDataFromFile(const char *filename) {

}

List* getCifarTrainDataList() {

}

void releaseCifarDataList(List* *cifarDataList) {
    if (cifarDataList != NULL) {

    }
}

static CifarData* loadCifarDataFromFile(const char *filename) {
    
}