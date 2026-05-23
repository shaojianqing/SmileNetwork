#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../datatype/stringtype.h"

#include "result.h"

static bool success(Result *this);

static Object* getData(Result *this);

static float getValue(Result *this);

static Result* createResultInner(int code, String *message, int type, Object *data, float value) {
    Result* result = (Result*)malloc(sizeof(Result));
    if (result != NULL) {
        result->success = success;
        result->getData = getData;

        result->code = code;
        result->message = message;
        result->type = type;
        result->data = data;
        result->value = value;
    }
    return result;
}

Result* createResultWithData(int code, String *message, int type, Object *data) {
    return createResultInner(code, message, type, data, 0.0);
}

Result* createResultWithValue(int code, String *message, float value) {
    return createResultInner(code, message, TYPE_FOLAT, NULL, value);
}

Result* createResultWithoutData(int code, String *message) {
    return createResultInner(code, message, TYPE_NONE, NULL, 0.0);
}

void releaseResult(Result* this) {
    if (this != NULL) {
        free(this->message);
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