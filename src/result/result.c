#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "result.h"

static bool success(Result *this);

static Object* getData(Result *this);

static float getValue(Result *this);

static Result* createResultInner(int code, char *message, int type, Object *data, float value) {
    Result* result = (Result*)malloc(sizeof(Result));
    if (result != NULL) {
        result->success = success;
        result->getData = getData;
        result->getValue = getValue;

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
        free(this);
    }
}

static bool success(Result *this) {
    return (this->code == SUCCESS);
}

static Object* getData(Result *this) {
    return this->data;
}

static float getValue(Result *this) {
    return this->value;
}