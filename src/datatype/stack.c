#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"

#include "stack.h"

struct Stack {

    int elementCapacity;

    int currentIndex;

    Object **dataList;
};

static bool resizeStack(Stack *this);

Stack* createStack(int initCapacity) {
    Stack *stack = (Stack*)allocate(sizeof(Stack));
    if (stack != NULL) {
        stack->currentIndex = -1;
        stack->elementCapacity = initCapacity;
        stack->dataList = allocate(sizeof(Object*) * initCapacity);
        if (stack->dataList == NULL) {
            release(stack);
            return NULL;
        }
    }
    return stack;
}

bool push(Stack *this, Object *element) {
    if (this->currentIndex == this->elementCapacity - 1) {
        bool success = resizeStack(this);
        if (!success) {
            return false;
        }
    }

    this->currentIndex++;
    this->dataList[this->currentIndex] = element;
    return true;
}

Object* pop(Stack *this) {
    if (this->currentIndex < 0) {
        return NULL;
    }
    Object *element = this->dataList[this->currentIndex];
    this->dataList[this->currentIndex] = NULL;
    this->currentIndex--;
    return element;
}

Object* peek(Stack *this) {
    if (this->currentIndex < 0) {
        return NULL;
    }
    return this->dataList[this->currentIndex];
}

int getStackCount(Stack *this) {
    if (this->currentIndex >= 0) {
        return this->currentIndex + 1;
    } else {
        return 0;
    }
}

static bool resizeStack(Stack *this) {
    Object **originDataList = this->dataList;
    this->dataList = allocate(this->elementCapacity * 2);
    if (this->dataList == NULL) {
        this->dataList = originDataList;
        return false;
    }

    for (int i=0;i<=this->currentIndex;++i) {
        this->dataList[i] = originDataList[i];
    }
    release(originDataList);
    return true;
}