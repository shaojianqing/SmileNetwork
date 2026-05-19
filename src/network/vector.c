#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"

#include "result.h"
#include "vector.h"

static void retain(Vector *this);

static void release(Vector *this);

static Result* mul(Vector *this, Vector *vector);

static Result* add(Vector *this, Vector *vector);

Vector *createVector(float *elements, int count) {
    Vector *vector = (Vector*)malloc(sizeof(Vector));
    if (vector != NULL) {
        vector->elements = elements;
        vector->count = count;

        vector->add = add;
        vector->mul = mul;

        vector->retain = retain;
        vector->release = release;
    }
    return vector;
}

static Result* mul(Vector *this, Vector *vector) {
    return NULL;
}

static Result* add(Vector *this, Vector *vector) {
    return NULL;
}

static void retain(Vector *this) {

}

static void release(Vector *this) {
    
}