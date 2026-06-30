#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../generator/generator.h"

#include "bias.h"
#include "vector.h"
#include "tensor.h"

#define MAX_DIMENSION           8

struct Tensor {

    int dimension;

    int elementSize;

    float *dataList;

    int shape[MAX_DIMENSION];

    int stride[MAX_DIMENSION];
};

static Exception MemoryAllocException = {MemoryAllocExceptionType};

static int calculateIndex(Tensor *this, const int index[]);

Tensor* createTensor(const int dimension, const int shape[], Generator generator) {
    assertTrue(dimension > MAX_DIMENSION, "tensor dimension should not be great than 8!");

    Tensor *tensor = (Tensor*)allocate(sizeof(Tensor));
    if (tensor == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for tensor creation.");
    }
    
    tensor->dimension = dimension;
    int totalDataCount = 1;
    for (int i=0;i<dimension;++i) {
        tensor->shape[i] = shape[i];
        totalDataCount *= tensor->shape[i];
    }
    tensor->elementSize = totalDataCount;

    tensor->stride[dimension - 1] = 1;
    for (int i=dimension-2;i>=0;--i) {
        tensor->stride[i] = tensor->stride[i+1] * tensor->shape[i+1];
    }

    tensor->dataList = (float*)allocate(sizeof(float)*totalDataCount);
    if (tensor->dataList == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for tensor data list creation.");
    }

    if (generator != NULL) {
        for (int i=0;i<tensor->dimension;++i) {
            tensor->dataList[i] = generator();
        }
    }

    return tensor;
}

void releaseTensor(Tensor *tensor) {
    if (tensor != NULL) {
        release(tensor->dataList);
        release(tensor);
    }
}

int getDimension(Tensor *this) {

    assertNotNull(this, "tensor instance is null for getting dimension!");
    return this->dimension;
}

int getElementSize(Tensor *this) {

    assertNotNull(this, "tensor instance is null for getting element size!");
    return this->elementSize;
}

int* getTensorShape(Tensor *this) {

    assertNotNull(this, "tensor instance is null for getting shape!");
    return this->shape;
}

float mulTensorScalar(Tensor *this, Tensor *target) {

    assertNotNull(this, "this tensor instance is null for hadamard multiplication!");
    assertNotNull(target, "target tensor instance is null for hadamard multiplication!");

    int thisSize = this->elementSize;
    int targetSize = target->elementSize;

    assertDataMatch(thisSize, targetSize, "element size does not match with this tensor and target tensor!");

    int thisDimension = this->dimension;
    int targetDimension = target->dimension;

    assertDataMatch(thisDimension, thisDimension, "dimension does not match with this tensor and target tensor!");

    int *thisShape = this->shape;
    int *targetShape = this->shape;

    for (int i=0;i<thisDimension;++i) {
        assertDataMatch(thisShape[i], targetShape[i], "shape does not match with this tensor and target tensor!");
    }

    float result = 0.0;
    for (int i=0;i<this->elementSize;++i) {
        result += this->dataList[i] * target->dataList[i];
    }
    return result;
}

void mulTensorNumber(Tensor *this, float number) {

    assertNotNull(this, "this tensor instance is null for hadamard multiplication!");

    int thisSize = this->elementSize;
    for (int i=0;i<this->elementSize;++i) {
        this->dataList[i] *= number;
    }
}

void addTensor(Tensor *this, Tensor *target) {

    assertNotNull(this, "this tensor instance is null for addition operation!");
    assertNotNull(target, "target tensor instance is null for addition operation!");

    int thisSize = this->elementSize;
    int targetSize = target->elementSize;

    assertDataMatch(thisSize, targetSize, "element size does not match with this tensor and target tensor!");

    int thisDimension = this->dimension;
    int targetDimension = target->dimension;

    assertDataMatch(thisDimension, thisDimension, "dimension does not match with this tensor and target tensor!");

    int *thisShape = this->shape;
    int *targetShape = this->shape;

    for (int i=0;i<thisDimension;++i) {
        assertDataMatch(thisShape[i], targetShape[i], "shape does not match with this tensor and target tensor!");
    }

    float result = 0.0;
    for (int i=0;i<this->elementSize;++i) {
        this->dataList[i] += target->dataList[i];
    }
}

void subTensor(Tensor *this, Tensor *target) {

    assertNotNull(this, "this tensor instance is null for subtraction operation!");
    assertNotNull(target, "target tensor instance is null for subtraction operation!");

    int thisSize = this->elementSize;
    int targetSize = target->elementSize;

    assertDataMatch(thisSize, targetSize, "element size does not match with this tensor and target tensor!");

    int thisDimension = this->dimension;
    int targetDimension = target->dimension;

    assertDataMatch(thisDimension, thisDimension, "dimension does not match with this tensor and target tensor!");

    int *thisShape = this->shape;
    int *targetShape = this->shape;

    for (int i=0;i<thisDimension;++i) {
        assertDataMatch(thisShape[i], targetShape[i], "shape does not match with this tensor and target tensor!");
    }

    float result = 0.0;
    for (int i=0;i<this->elementSize;++i) {
        this->dataList[i] -= target->dataList[i];
    }
}

Vector* flattenTenosr(Tensor *this) {

    assertNotNull(this, "tensor instance is null for flattening operation!");

    Vector *result = createVector(this->elementSize);
    for (int i=0;i<this->elementSize;++i) {
        float value = this->dataList[i];
        setVectorValue(result, i, value);
    }

    return result;
}

float getTensorValue(Tensor *this, const int index[]) {
     
    assertNotNull(this, "tensor instance is null for getting value!");

    int position = calculateIndex(this, index);
    return this->dataList[position];
}

void setTensorValue(Tensor *this, const int index[], float value) {

    assertNotNull(this, "tensor instance is null for setting value!");

    int position = calculateIndex(this, index);
    this->dataList[position] = value;
}

static int calculateIndex(Tensor *this, const int index[]) {

    assertNotNull(this, "tensor instance is null for calculating tensor index!");

    int position = 0;
    for (int i=0;i<this->dimension;++i) {
        position += index[i] * this->stride[i];
    }
    return position;
}