#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../result/result.h"

#include "bias.h"
#include "vector.h"
#include "matrix.h"

static Result* mulVector(Matrix *this, Vector *vector);

static Result* addMatrix(Matrix *this, Matrix *metrix);

static Result* subMatrix(Matrix *this, Matrix *matrix);

static Result* mulMatrix(Matrix *this, Matrix *metrix);

static Result* mulNumber(Matrix *this, float number);

static Result* transpose(Matrix *this);

static int calculateIndex(Matrix *this, int row, int column);

static float getElementValue(Matrix *this, int row, int column);

static void setElementValue(Matrix *this, int row, int column, float data);

Matrix* createMatrix(int rowCount, int columnCount, Random random) {
    Matrix *matrix = (Matrix*)allocate(sizeof(Matrix));
    if (matrix != NULL) {
        matrix->transpose = transpose;
        matrix->addMatrix = addMatrix;
        matrix->subMatrix = subMatrix;
        matrix->mulVector = mulVector;
        matrix->mulMatrix = mulMatrix;
        matrix->mulNumber = mulNumber;
        matrix->setValue = setElementValue;

        matrix->rowCount = rowCount;
        matrix->columnCount = columnCount;
        matrix->data = (float *)allocate(sizeof(float)*rowCount*columnCount);

        if (random != NULL) {
            int totalCount = rowCount*columnCount;
            for (int i=0;i<totalCount;++i) {
                matrix->data[i] = random();
            }
        }
    }
    return matrix;
}

void releaseMatrix(Matrix *matrix) {
    if (matrix != NULL) {
        release(matrix->data);
        release(matrix);
    }
}

static Result* mulVector(Matrix *this, Vector *vector) {
    if (this->columnCount != vector->count) {
        char *message = "matrix column count does not match vector element count^o^";
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    Vector *resultVector = createVector(this->rowCount);
    if (resultVector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<this->rowCount;++i) {
        float sum = 0.0;
        for (int j=0;j<this->columnCount;++j) {
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
        char *message = "matrix instance is null for addition operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->rowCount != matrix->rowCount || this->columnCount != matrix->columnCount) {
        char *message = "matrix row count or column count does not match^o^";
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
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
        char *message = "matrix instance is null for subtraction operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->rowCount != matrix->rowCount || this->columnCount != matrix->columnCount) {
        char *message = "matrix row count or column count does not match^o^";
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float thisValue = getElementValue(this, i, j);
            float matrixValue = getElementValue(matrix, i, j);
            
            thisValue -= matrixValue;
            setElementValue(this, i, j, thisValue);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* mulMatrix(Matrix *this, Matrix *matrix) {
    if (this == NULL || matrix == NULL) {
        char *message = "matrix instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->columnCount != matrix->rowCount) {
        char *message = "matrix row count or column count does not match^o^";
        return createResultWithoutData(MATRIX_NOT_MATCH, message);
    }

    Matrix *resultMatrix = createMatrix(this->rowCount, matrix->columnCount, NULL);
    if (resultMatrix == NULL) {
        char *message = "can not create matrix instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int k=0;k<matrix->columnCount;++k) {
        for (int i=0;i<this->rowCount;++i) {
            float sum = 0.0;
            for (int j=0;j<this->columnCount;++j) {
                float thisValue = getElementValue(this, i, j);
                float matrixValue = getElementValue(matrix, j, k);
                sum += thisValue*matrixValue;
                setElementValue(resultMatrix, i, k, sum);
            }
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_METRIX, resultMatrix);
}

static Result* mulNumber(Matrix *this, float number) {
    if (this == NULL) {
        char *message = "matrix instance is null for matrix number multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float value = getElementValue(this, i, j);
            setElementValue(this, i, j, value * number);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

static Result* transpose(Matrix *this) {
    if (this == NULL) {
        char *message = "matrix instance is null for matrix tranpose operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    Matrix *transposeMatrix = createMatrix(this->columnCount, this->rowCount, NULL);
    if (transposeMatrix == NULL) {
        char *message = "can not create matrix instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float value = getElementValue(this, i, j);
            transposeMatrix->setValue(transposeMatrix, j, i, value);
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_METRIX, transposeMatrix);
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
