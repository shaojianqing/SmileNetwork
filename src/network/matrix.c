#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"

#include "result.h"
#include "vector.h"
#include "matrix.h"

static Result* mulVector(Matrix *this, Vector *vector);

static Result* addMatrix(Matrix *this, Matrix *metrix);

static Result* mulMatrix(Matrix *this, Matrix *metrix);

static void retain(Matrix *this);

static void release(Matrix *this);

Matrix* createMatrix() {
    Matrix *matrix = (Matrix*)malloc(sizeof(Matrix));
    if (matrix != NULL) {
        matrix->mulVector = mulVector;
        matrix->addMatrix = addMatrix;
        matrix->mulMatrix = mulMatrix;

        matrix->retain = retain;
        matrix->release = release;
    }
    return matrix;
}

static Result* mulVector(Matrix *this, Vector *vector) {
    return NULL;
}

static Result* addMatrix(Matrix *this, Matrix *metrix) {
    return NULL;
}

static Result* mulMatrix(Matrix *this, Matrix *metrix) {
    return NULL;
}

static void retain(Matrix *this) {

}

static void release(Matrix *this) {

}
