#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "result.h"

static bool success(Result *this);

static void retain(Result *this);

static void release(Result *this);

Result* createResult(int code, char *message, int type, ResulData data) {
    Result* result = (Result*)malloc(sizeof(Result));
    if (result != NULL) {

        result->success = success;
        result->retain = retain;
        result->release = release;

        result->code = code;
        result->message = message;
        result->type = type;
    }

     return result;
}

static bool success(Result *this) {
    return true;
}

static void retain(Result *this) {

}

static void release(Result *this) {
    
}