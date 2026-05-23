#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../datatype/stringtype.h"

#include "bias.h"
#include "result.h"
#include "vector.h"
#include "matrix.h"

static Result* mulVector(Matrix *this, Vector *vector);

static Result* addMatrix(Matrix *this, Matrix *metrix);

static Result* subMatrix(Matrix *this, Matrix *matrix);

static Result* mulMatrix(Matrix *this, Matrix *metrix);

static int calculateIndex(Matrix *this, int row, int column);

static float getElementValue(Matrix *this, int row, int column);

static void setElementValue(Matrix *this, int row, int column, float data);

Matrix* createMatrix(int rowCount, int columnCount) {
    Matrix *matrix = (Matrix*)malloc(sizeof(Matrix));
    if (matrix != NULL) {
        matrix->mulVector = mulVector;
        matrix->addMatrix = addMatrix;
        matrix->subMatrix = subMatrix;
        matrix->mulMatrix = mulMatrix;

        matrix->data = (float*)malloc(sizeof(float)*rowCount*columnCount);

    }
    return matrix;
}

void releaseMatrix(Matrix *matrix) {
    if (matrix != NULL) {
        free(matrix->data);
        free(matrix);
    }
}

static Result* mulVector(Matrix *this, Vector *vector) {
    if (this->columnCount != vector->count) {
        String *message = createString("matrix column count does not match vector element count^o^");
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    Vector *resultVector = createVector(this->columnCount);
    if (resultVector == NULL) {
        String *message = createString("can not create vector instance for memory allocation error^o^");
        return createResultWithoutData(MEMORY_ALLOCATE_ERROR, message);
    }

    int i = 0, j = 0;
    for (i=0;i<this->rowCount;++i) {
        float sum = 0.0;
        for (j=0;j<this->columnCount;++j) {
            float matrixValue = getElementValue(this, i, j);
            float vectorValue = vector->getValue(vector, j);
            sum += matrixValue*vectorValue;
            resultVector->setValue(resultVector, i, sum);
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, resultVector);
}

static Result* addMatrix(Matrix *this, Matrix *matrix) {
    if (this == NULL || matrix == NULL) {
        String *message = createString("matrix instance is null for addition operation^o^");
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->rowCount != matrix->rowCount || this->columnCount != matrix->columnCount) {
        String *message = createString("matrix row count or column count does not match^o^");
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    int i = 0, j = 0;
    for (i=0;i<this->rowCount;++i) {
        for (j=0;j<this->columnCount;++j) {
            float matrixValue = getElementValue(matrix, i, j);
            float thisValue = getElementValue(this, i, j);

            thisValue += matrixValue;
            setElementValue(this, i, j, thisValue);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* subMatrix(Matrix *this, Matrix *matrix) {
    if (this == NULL || matrix == NULL) {
        String *message = createString("matrix instance is null for subtraction operation^o^");
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->rowCount != matrix->rowCount || this->columnCount != matrix->columnCount) {
        String *message = createString("matrix row count or column count does not match^o^");
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    int i = 0, j = 0;
    for (i=0;i<this->rowCount;++i) {
        for (j=0;j<this->columnCount;++j) {
            float matrixValue = getElementValue(matrix, i, j);
            float thisValue = getElementValue(this, i, j);

            thisValue -= matrixValue;
            setElementValue(this, i, j, thisValue);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* mulMatrix(Matrix *this, Matrix *matrix) {
    if (this == NULL || matrix == NULL) {
        String *message = createString("matrix instance is null for multiplication operation^o^");
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->columnCount != matrix->rowCount) {
        String *message = createString("matrix row count or column count does not match^o^");
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    Matrix *resultMatrix = createMatrix(this->rowCount, matrix->columnCount);
    if (resultMatrix == NULL) {
        String *message = createString("can not create matrix instance for memory allocation error^o^");
        return createResultWithoutData(MEMORY_ALLOCATE_ERROR, message);
    }

    int i = 0, j = 0, k = 0;
    for (k=0;k<matrix->columnCount;++k) {
        for (i=0;i<this->rowCount;++i) {
            float sum = 0.0;
            for (j=0;j<this->columnCount;++j) {
                float thisValue = getElementValue(this, i, j);
                float matrixValue = getElementValue(this, j, k);
                sum += thisValue*matrixValue;
                setElementValue(resultMatrix, i, k, sum);
            }
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_METRIX, resultMatrix);
}

static int calculateIndex(Matrix *this, int row, int column) {
    int columnCount = this->columnCount;
    return row*columnCount + column;
}

static float getElementValue(Matrix *this, int row, int column) {
    int index = calculateIndex(this, row, column);
    return this->data[index];
}

static void setElementValue(Matrix *this, int row, int column, float value) {
    int index = calculateIndex(this, row, column);
    this->data[index] = value;
}
