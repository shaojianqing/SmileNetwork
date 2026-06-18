#include <stdio.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../except/exception.h"
#include "../except/assertion.h"

#include "bias.h"
#include "vector.h"
#include "matrix.h"

struct Vector {

    float *elements;

    int count;
};

static Exception MemoryAllocException = {MemoryAllocExceptionType};

Vector* createVector(int count) {
    Vector *vector = (Vector*)allocate(sizeof(Vector));
    if (vector != NULL) {
        vector->elements = (float *)allocate(sizeof(float)*count);
        vector->count = count;
    } else {
        throw(&MemoryAllocException, "can not allocate memory for vector creation.");
    }
    return vector;
}

void releaseVector(Vector *this) {
    if (this != NULL) {
        release(this->elements);
        release(this);
    }
}

int getElementCount(Vector *this) {
    if (this != NULL) {
        return this->count;
    }
    return 0;
}

float mulScalar(Vector *this, Vector *target) {

    assertNotNull(this, "vector instance is null for multiplication scalar operation!");
    assertNotNull(target, "vector instance is null for multiplication scalar operation!");

    assertDataMatch(this->count, target->count, "vector count does not match for multiplication scalar operation!");

    float sum = 0.0;
    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float targetValue = target->elements[i];
        sum += thisValue * targetValue;
    }
    return sum;
}

Matrix* mulTensor(Vector *this, Vector *target) {

    assertNotNull(this, "vector instance is null for tensor multiplication operation!");
    assertNotNull(target, "vector instance is null for tensor multiplication operation!");

    Matrix *resultMatrix = createMatrix(this->count, target->count, NULL);
    for (int i=0;i<this->count;++i) {
        for (int j=0;j<target->count;++j) {
            float thisValue = this->elements[i];
            float targetValue = target->elements[j];
            float resultValue = thisValue * targetValue;
            setMatrixValue(resultMatrix, i, j, resultValue);
        }
    }
    return resultMatrix;
}

Vector* mulHadamard(Vector *this, Vector *target) {

    assertNotNull(this, "vector instance is null for hadamard multiplication operation!");
    assertNotNull(target, "vector instance is null for hadamard multiplication operation!");

    assertDataMatch(this->count, target->count, "vector count does not match for multiplication hadamard operation!");

    Vector *resultVector = createVector(this->count);
    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float targetValue = target->elements[i];
        resultVector->elements[i] = thisValue * targetValue;
    }
    return resultVector;
}

Vector* mulMatrixVector(Vector *this, Matrix *matrix) {

    assertNotNull(this, "vector instance is null for multiplication between vector and matrix operation!");
    assertNotNull(matrix, "matrix instance is null for multiplication between vector and matrix operation!");

    assertDataMatch(this->count, getRowCount(matrix), "vector and matrix does not match for multiplication between vector and matrix operation!");

    int vectorCount = getColumnCount(matrix);
    Vector *resultVector = (Vector*)createVector(vectorCount);
    for (int i=0;i<vectorCount;++i) {
        float resultValue = 0.0;
        for (int j=0;j<this->count;++j) {
            float vectorValue = getVectorValue(this, j);
            float matrixValue = getMatrixValue(matrix, j, i);
            resultValue += vectorValue * matrixValue;
        }
        setVectorValue(resultVector, i, resultValue);
    }
    return resultVector;
}

void addVector(Vector *this, Vector *target) {

    assertNotNull(this, "vector instance is null for vector addition operation!");
    assertNotNull(target, "vector instance is null for vector addition operation!");

    assertDataMatch(this->count, target->count, "vector count does not match for vector addition operation!");

    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float targetValue = target->elements[i];

        this->elements[i] = thisValue + targetValue;
    }
}

void addBias(Vector *this, Bias *bias) {

    assertNotNull(this, "vector instance is null for vector addition operation!");
    assertNotNull(bias, "bias instance is null for vector addition operation!");

    assertDataMatch(this->count, getBiasElementCount(bias), "vector count does not match for vector and bias addition operation!");

    for (int i=0;i<this->count;++i) {
        float thisValue = getVectorValue(this, i);
        float biasValue = getBiasValue(bias, i);

        float totalValue = thisValue + biasValue;
        setVectorValue(this, i, totalValue);
    }
}

void copyVector(Vector *this, Vector *target) {

    assertNotNull(this, "vector instance is null for vector copy operation!");
    assertNotNull(target, "vector instance is null for vector copy operation!");

    assertDataMatch(this->count, target->count, "vector count does not match for vector copy operation!");

    for (int i=0;i<this->count;++i) {
        float targetValue = target->elements[i];
        this->elements[i] = targetValue;
    }
}

float getVectorValue(Vector *this, int index) {
    if (index<this->count) {
        return this->elements[index];
    }
    return 0.0;
}

void setVectorValue(Vector *this, int index, float value) {
    if (index<this->count) {
        this->elements[index] = value;
    }
}
