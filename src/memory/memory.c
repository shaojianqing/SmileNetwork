#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"

#include "memory.h"

Object* allocate(int size) {
    Object *ptr = malloc(size);
    if (ptr != NULL) {
        memset(ptr, size, 1);
    }
    return ptr;
}

void release(Object* ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}
