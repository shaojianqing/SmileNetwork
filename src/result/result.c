#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../common/constant.h"

#include "result.h"

struct Result {

    int code;

    int type;

    char *message;

    Object *data;

    float value;
};

static Result* createResultInner(int code, char *message, int type, Object *data, float value) {
    Result* result = (Result*)allocate(sizeof(Result));
    if (result != NULL) {
        result->code = code;
        result->message = message;
        result->type = type;
        result->data = data;
        result->value = value;
    }
    return result;
}

Result* createResultWithData(int code, char *message, int type, Object *data) {
    return createResultInner(code, message, type, data, 0.0);
}

Result* createResultWithValue(int code, char *message, float value) {
    return createResultInner(code, message, TYPE_FLOAT, NULL, value);
}

Result* createResultWithoutData(int code, char *message) {
    return createResultInner(code, message, TYPE_NONE, NULL, 0.0);
}

void releaseResult(Result* this) {
    if (this != NULL) {
        release(this);
    }
}

bool success(Result *this) {
    return (this->code == SUCCESS);
}

int getCode(Result *this) {
    return this->code;
}

char* getMessage(Result *this) {
    return this->message;
}

int getType(Result *this) {
    return this->type;
}

Object* getData(Result *this) {
    return this->data;
}

float getValue(Result *this) {
    return this->value;
}