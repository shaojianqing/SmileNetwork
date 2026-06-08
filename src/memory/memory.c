#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../logger/logger.h"
#include "../datatype/datatype.h"
#include "../datatype/stringtype.h"
#include "../datatype/hashmap.h"
#include "../printer/printer.h"

#include "memory.h"

extern Logger logger;

static HashMap *allocationTable;

void initMemoryConfig() {
    allocationTable = createHashMap(StringHashCode, StringEqualFun, 100);
}

void printAllocationTable() {
    for (int i=0;i<allocationTable->capacity;++i) {
        Entry *entry = allocationTable->table[i];
        while (entry != NULL) {
            String *key = (String*)entry->key;
            String *value = (String*)entry->value;
            printMessage(RED, "[%s =====> %s]", key->getValue(key), value->getValue(value));
            entry = entry->next;			
        }
    }
}

Object* alloc(char *file, int line, int size) {
    Object *ptr = malloc(size);
    if (ptr != NULL) {
        String *address = createStringWithFormat("%p", (Object *)ptr);
        String *identifier = createStringWithFormat("[%s:%i]", file, line);

        allocationTable->put(allocationTable, address, identifier);
    }
    return ptr;
}

void release(Object* ptr) {

    String *address = createStringWithFormat("%p", (Object *)ptr);
    if (allocationTable->containsKey(allocationTable, address)) {
        String *value = allocationTable->get(allocationTable, address);
        allocationTable->remove(allocationTable, address);
        releaseString(value);
    }
    releaseString(address);
    free(ptr);
}