#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"

#include "bias.h"
#include "vector.h"
#include "matrix.h"

struct Matrix {

    float* data;

    int rowCount;

    int columnCount;
};

static Exception MemoryAllocException = {MemoryAllocExceptionType};

static int calculateIndex(Matrix *this, int row, int column);

Matrix* createMatrix(int rowCount, int columnCount, Generator generator) {
    Matrix *matrix = (Matrix*)allocate(sizeof(Matrix));
    if (matrix != NULL) {
        matrix->data = (float *)allocate(sizeof(float)*rowCount*columnCount);
        matrix->rowCount = rowCount;
        matrix->columnCount = columnCount;
        
        if (generator != NULL) {
            int totalCount = rowCount*columnCount;
            for (int i=0;i<totalCount;++i) {
                matrix->data[i] = generator();
            }
        }
    } else {
        throw(&MemoryAllocException, "can not allocate memory for matrix creation.");
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

Vector* mulVector(Matrix *this, Vector *vector) {

    assertNotNull(this, "matrix instance is null for matrix and vector multiplication operation!");
    assertNotNull(vector, "vector instance is null for matrix and vector multiplication operation!");

    assertDataMatch(this->columnCount, getElementCount(vector), "matrix column count does not match vector element count for matrix and vector multiplication operation!");

    Vector *resultVector = createVector(this->rowCount);
    for (int i=0;i<this->rowCount;++i) {
        float sum = 0.0;
        for (int j=0;j<this->columnCount;++j) {
            float matrixValue = getMatrixValue(this, i, j);
            float vectorValue = getVectorValue(vector, j);
            sum += matrixValue * vectorValue;
            setVectorValue(resultVector, i, sum);
        }
    }
    return resultVector;
}

void addMatrix(Matrix *this, Matrix *matrix) {
    
    assertNotNull(this, "matrix instance is null for matrix addition operation!");
    assertNotNull(matrix, "matrix instance is null for matrix addition operation!");

    assertDataMatch(this->rowCount, matrix->rowCount, "matrix row count does not match for matrix addition operation!");
    assertDataMatch(this->columnCount, matrix->columnCount, "matrix column count does not match for matrix addition operation!");

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float matrixValue = getMatrixValue(matrix, i, j);
            float thisValue = getMatrixValue(this, i, j);

            thisValue += matrixValue;
            setMatrixValue(this, i, j, thisValue);
        }
    }
}

void subMatrix(Matrix *this, Matrix *matrix) {
    
    assertNotNull(this, "matrix instance is null for matrix subtraction operation!");
    assertNotNull(matrix, "matrix instance is null for matrix subtraction operation!");

    assertDataMatch(this->rowCount, matrix->rowCount, "matrix row count does not match for matrix subtraction operation!");
    assertDataMatch(this->columnCount, matrix->columnCount, "matrix column count does not match for matrix subtraction operation!");

    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float thisValue = getMatrixValue(this, i, j);
            float matrixValue = getMatrixValue(matrix, i, j);
            
            thisValue -= matrixValue;
            setMatrixValue(this, i, j, thisValue);
        }
    }
}

Matrix* mulMatrix(Matrix *this, Matrix *matrix) {
    
    assertNotNull(this, "matrix instance is null for matrix multiplication operation!");
    assertNotNull(matrix, "matrix instance is null for matrix multiplication operation!");

    assertDataMatch(this->columnCount, matrix->rowCount, "this column count does not match matrix row count for matrix multiplication operation!");

    Matrix *resultMatrix = createMatrix(this->rowCount, matrix->columnCount, NULL);
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
    return resultMatrix;
}

void mulMatrixNumber(Matrix *this, float number) {

    assertNotNull(this, "matrix instance is null for matrix number multiplication operation!");
    for (int i=0;i<this->rowCount;++i) {
        for (int j=0;j<this->columnCount;++j) {
            float value = getMatrixValue(this, i, j);
            setMatrixValue(this, i, j, value * number);
        }
    }
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