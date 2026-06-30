#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"

#include "stack.h"

typedef struct StackImpl {

    Stack stack;

    int elementCapacity;

    int currentIndex;

    Object **dataList;

} StackImpl;

static bool push(Stack *this, Object *element);

static Object* pop(Stack *this);

static Object* peek(Stack *this);

static int getStackCount(Stack *this);

static bool resizeStack(StackImpl *this);

Stack* createStack(int initCapacity) {
    StackImpl *stackImpl = (StackImpl*)allocate(sizeof(Stack));
    if (stackImpl != NULL) {
        Stack *stack = (Stack*)stackImpl;
        stack->push = push;
        stack->pop = pop;
        stack->peek = peek;
        stack->getStackCount = getStackCount;

        stackImpl->currentIndex = -1;
        stackImpl->elementCapacity = initCapacity;
        stackImpl->dataList = allocate(sizeof(Object*) * initCapacity);
        if (stackImpl->dataList == NULL) {
            release(stackImpl);
            return NULL;
        }
    }
    return (Stack*)stackImpl;
}

void releaseStack(Stack *stack) {
    if (stack != NULL) {
        StackImpl *stackImpl = (StackImpl*)stack;
        release(stackImpl->dataList);
        release(stackImpl);
    }
}

static bool push(Stack *this, Object *element) {
    if (this != NULL) {
        StackImpl *stackImpl = (StackImpl*)this;
        if (stackImpl->currentIndex == stackImpl->elementCapacity - 1) {
            bool success = resizeStack(stackImpl);
            if (!success) {
                return false;
            }
        }
        stackImpl->currentIndex++;
        stackImpl->dataList[stackImpl->currentIndex] = element;
        return true;
    }
    return false;
}    

static Object* pop(Stack *this) {
    if (this != NULL) {
        StackImpl *stackImpl = (StackImpl*)this;
        if (stackImpl->currentIndex < 0) {
            return NULL;
        }
        Object *element = stackImpl->dataList[stackImpl->currentIndex];
        stackImpl->dataList[stackImpl->currentIndex] = NULL;
        stackImpl->currentIndex--;
        return element;
    }
    return NULL;
}

static Object* peek(Stack *this) {
    if (this != NULL) {
        StackImpl *stackImpl = (StackImpl*)this;
        if (stackImpl->currentIndex < 0) {
            return NULL;
        }
        return stackImpl->dataList[stackImpl->currentIndex];
    }
    return NULL;
}

static int getStackCount(Stack *this) {
    if (this != NULL) {
        StackImpl *stackImpl = (StackImpl*)this;
        if (stackImpl->currentIndex >= 0) {
            return stackImpl->currentIndex + 1;
        } else {
            return 0;
        }
    }
    return 0;
}

static bool resizeStack(StackImpl *this) {
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