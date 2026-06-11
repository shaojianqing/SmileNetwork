#include <stdio.h>
#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"
#include "../random/random.h"
#include "../result/result.h"

#include "bias.h"
#include "vector.h"
#include "matrix.h"

struct Vector {

    float *elements;

    int count;
};

Vector *createVector(int count) {
    Vector *vector = (Vector*)allocate(sizeof(Vector));
    if (vector != NULL) {
        vector->elements = (float *)allocate(sizeof(float)*count);
        vector->count = count;
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

Result* mulScalar(Vector *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "vector instance is null for multiplication scalar operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        char *message = "vector does not match for multiplication scalar operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    float sum = 0.0;
    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float vectorValue = vector->elements[i];
        sum += thisValue * vectorValue;
    }
    return createResultWithValue(SUCCESS, NULL, sum);
}

Result* mulTensor(Vector *this, Vector *target) {
    if (this == NULL || target == NULL) {
        char *message = "vector instance is null for matrix multiplication tensor operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    Matrix *resultMatrix = createMatrix(this->count, target->count, NULL);
    if (resultMatrix == NULL) {
        char *message = "can not create matrix instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<this->count;++i) {
        for (int j=0;j<target->count;++j) {
            float thisValue = this->elements[i];
            float targetValue = target->elements[j];
            float resultValue = thisValue * targetValue;
            setMatrixValue(resultMatrix, i, j, resultValue);
        }
    }
    return createResultWithData(SUCCESS, NULL, TYPE_METRIX, resultMatrix);
}

Result* mulHadamard(Vector *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "vector instance is null for multiplication hadamard operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        char *message = "vector does not match for multiplication hadamard operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    Vector *resultVector = createVector(this->count);
    if (resultVector == NULL) {
        char *message = "can not create vector instance for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float vectorValue = vector->elements[i];
        resultVector->elements[i] = thisValue * vectorValue;
    }
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, resultVector);
}

Result* mulMatrixVector(Vector *this, Matrix *matrix) {
    if (this == NULL || matrix == NULL) {
        char *message = "vector or matrix instance is null for multiplication matrix operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != getRowCount(matrix)) {
        char *message = "vector and matrix does not match for multiplication hadamard operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

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
    return createResultWithData(SUCCESS, NULL, TYPE_VECTOR, resultVector);
}

Result* addVector(Vector *this, Vector *vector) {
    if (this == NULL || vector == NULL) {
        char *message = "vector instance is null for addition operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != vector->count) {
        char *message = "vector does not match for addition operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float thisValue = this->elements[i];
        float vectorValue = vector->elements[i];

        this->elements[i] = thisValue + vectorValue;
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* addBias(Vector *this, Bias *bias) {
    if (this == NULL || bias == NULL) {
        char *message = "vector or bias instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != getBiasElementCount(bias)) {
        char *message = "vector and bias does not match for multiplication operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float thisValue = getVectorValue(this, i);
        float biasValue = getBiasValue(bias, i);

        float totalValue = thisValue + biasValue;
        setVectorValue(this, i, totalValue);
    }
    return createResultWithoutData(SUCCESS, NULL);
}

Result* copyVector(Vector *this, Vector *target) {
    if (this == NULL || target == NULL) {
        char *message = "vector instance is null for multiplication operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    if (this->count != target->count) {
        char *message = "vector does not match for multiplication operation^o^";
        return createResultWithoutData(VECTOR_NOT_MATCH, message);
    }

    for (int i=0;i<this->count;++i) {
        float targetValue = target->elements[i];
        this->elements[i] = targetValue;
    }
    return createResultWithoutData(SUCCESS, NULL);
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
