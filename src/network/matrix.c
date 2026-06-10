#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../result/result.h"

#include "bias.h"
#include "vector.h"
#include "matrix.h"

struct Matrix {

    float* data;

    int rowCount;

    int columnCount;
};

static int calculateIndex(Matrix *this, int row, int column);

Matrix* createMatrix(int rowCount, int columnCount, Random random) {
    Matrix *matrix = (Matrix*)allocate(sizeof(Matrix));
    if (matrix != NULL) {
        matrix->data = (float *)allocate(sizeof(float)*rowCount*columnCount);
        matrix->rowCount = rowCount;
        matrix->columnCount = columnCount;
        
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

int getRowCount(Matrix *this) {
    if (this != NULL) {
        return this->rowCount;
    }
    return 0;
}

int getColumnCount(Matrix *this) {
    if (this != NULL) {
        return this->columnCount;
    }
    return 0;
}

Result* mulVector(Matrix *this, Vector *vector) {
    if (this->columnCount != getElementCount(vector)) {
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
            float matrixValue = getMatrixValue(this, i, j);
            float vectorValue = getVectorValue(vector, j);
            sum += matrixValue * vectorValue;
            setVectorValue(resultVector, i, sum);
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, resultVector);
}

Result* addMatrix(Matrix *this, Matrix *matrix) {
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
            float matrixValue = getMatrixValue(matrix, i, j);
            float thisValue = getMatrixValue(this, i, j);

            thisValue += matrixValue;
            setMatrixValue(this, i, j, thisValue);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* subMatrix(Matrix *this, Matrix *matrix) {
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
            float thisValue = getMatrixValue(this, i, j);
            float matrixValue = getMatrixValue(matrix, i, j);
            
            thisValue -= matrixValue;
            setMatrixValue(this, i, j, thisValue);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* mulMatrix(Matrix *this, Matrix *matrix) {
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
                float thisValue = getMatrixValue(this, i, j);
                float matrixValue = getMatrixValue(matrix, j, k);
                sum += thisValue * matrixValue;
                setMatrixValue(resultMatrix, i, k, sum);
            }
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_METRIX, resultMatrix);
}

Result* mulMatrixNumber(Matrix *this, float number) {
    if (this == NULL) {
        char *message = "matrix instance is null for matrix number multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float value = getMatrixValue(this, i, j);
            setMatrixValue(this, i, j, value * number);
        }
    }
    return createResultWithoutData(SUCCESS, NULL);
}

float getMatrixValue(Matrix *this, int row, int column) {
    int index = calculateIndex(this, row, column);
    return this->data[index];
}

void setMatrixValue(Matrix *this, int row, int column, float value) {
    int index = calculateIndex(this, row, column);
    this->data[index] = value;
}

static int calculateIndex(Matrix *this, int row, int column) {
    int columnCount = this->columnCount;
    return row*columnCount + column;
}