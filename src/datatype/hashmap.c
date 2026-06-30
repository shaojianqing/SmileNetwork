#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "datatype.h"
#include "hashmap.h"

#define LIMIT_FACTOR 0.75

typedef struct Entry Entry;

typedef struct HashMap HashMap;

struct Entry {
	
	Object *key;
	
	Object *value;

	Entry *next;
};

struct HashMap {

	Map map;
	
	Entry **table;

	int count;

	int limit;

	int capacity;

	HashCode hashCode;

	EqualFunc equalFunc;
};

static int put(Map *this, Object *key, Object *value);

static Object* get(Map *this, Object *key);

static int removeByKey(Map *this, Object *key);

static bool containsKey(Map *this, Object *key);

static int getCount(Map *this);

static int reHash(HashMap *this);

static int putInner(HashMap *this, Object *key, Object *value);

static void reBuildMap(HashMap *this, Entry **table, Entry *entry, int capacity);

static Entry* getEntryByKey(HashMap *this, Object *key);

Map* createHashMap(HashCode hashCode, EqualFunc equalFunc, int capacity) {
	if (hashCode != NULL && equalFunc != NULL && capacity > 0) {
		HashMap *hashMap = (HashMap *)malloc(sizeof(HashMap));
		if (hashMap != NULL) {
			Map *map = (Map*)hashMap;
			map->containsKey = containsKey;
			map->remove = removeByKey;
			map->getCount = getCount;
			map->put = put;
			map->get = get;	

			hashMap->count = 0;
			hashMap->capacity = capacity;
			hashMap->table = malloc(sizeof(Entry *)*capacity);
			memset(hashMap->table, 0, sizeof(Entry *)*capacity);
			hashMap->limit = capacity * LIMIT_FACTOR;
			hashMap->hashCode = hashCode;
			hashMap->equalFunc = equalFunc;		
		}
		return (Map*)hashMap;
	} else {
		return NULL;	
	}
}

static int put(Map *this, Object *key, Object *value) {
	if (this != NULL && key != NULL && value != NULL) {
		HashMap *hashMap = (HashMap*)this;
		if (hashMap->count >= hashMap->limit) {
			if (reHash(hashMap)==STATUS_SUCCESS) {
				if (putInner(hashMap, key, value)) {
					hashMap->count++;
					return STATUS_SUCCESS;				
				}
			}		
		} else {
			if (putInner(hashMap, key, value)) {
				hashMap->count++;
				return STATUS_SUCCESS;				
			}	
		}		
	}
	return STATUS_FAILURE;
}

static Object* get(Map *this, Object *key) {
	if (this != NULL && key != NULL) {
		HashMap *hashMap = (HashMap*)this;
		int hashCode = hashMap->hashCode(key);
		int index = (hashCode & (hashMap->capacity-1));
		Entry *entry = hashMap->table[index];
		while(entry != NULL) {
			if (hashMap->equalFunc(entry->key, key)) {
				return entry->value;			
			}
			entry = entry->next;		
		}
	}
	return NULL;
}

static int getCount(Map *this) {
	if (this != NULL) {
		HashMap *hashMap = (HashMap*)this;
		return hashMap->count;	
	} else {
		return 0;
	}
}

static int removeByKey(Map *this, Object *key) {
	if (this != NULL && key != NULL) {
		HashMap *hashMap = (HashMap*)this;
		int hashCode = hashMap->hashCode(key);
		int index = (hashCode & (hashMap->capacity-1));
		Entry *entry = hashMap->table[index];
		if (entry!=NULL) {
			if (hashMap->equalFunc(entry->key, key)) {
				hashMap->table[index] = entry->next;
				free(entry);
				hashMap->count--;
				return STATUS_SUCCESS;
			} else {
				Entry *prev = entry;
				entry = entry->next;
				while (entry != NULL) {
					if (hashMap->equalFunc(entry->key, key)) {
						prev->next = entry->next;
						free(entry);
						hashMap->count--;
						return STATUS_SUCCESS; 
					} else {
						prev = entry;
						entry = entry->next;
					}					
				}			
			}
		}		
	}
	return STATUS_FAILURE;
}

static bool containsKey(Map *this, Object *key) {
	if (this != NULL && key != NULL) {
		HashMap *hashMap = (HashMap*)this;
		int hashCode = hashMap->hashCode(key);
		int index = (hashCode & (hashMap->capacity-1));
		Entry *entry = hashMap->table[index];
		while (entry!=NULL) {
			if (hashMap->equalFunc(entry->key, key)) {
				return true;			
			}
			entry = entry->next;
		}
	}
	return false;
}

static int reHash(HashMap *this) {
	if (this!=NULL) {
		int oldCapacity = this->capacity;
		int newCapacity = this->capacity*2;
		Entry **oldTable = this->table;
		Entry **newTable = malloc(sizeof(Entry *)*newCapacity);
		memset(newTable, 0, sizeof(Entry *)*newCapacity);
		if (newTable!=NULL) {
			for (int i=0;i<oldCapacity;++i) {
				Entry *entry = oldTable[i];
				Entry *nextEntry = NULL;
				oldTable[i] = NULL;
				while (entry!=NULL) {
					nextEntry = entry->next;		
					reBuildMap(this, newTable, entry, newCapacity);
					entry = nextEntry;
				}		
			}
			this->table = newTable;
			this->capacity = newCapacity;
			this->limit = newCapacity*LIMIT_FACTOR;
			free(oldTable);	
			return STATUS_SUCCESS;			
		}
	}	
	return STATUS_SUCCESS;
}

static int putInner(HashMap *this, Object *key, Object *value) {
	Entry *entry = getEntryByKey(this, key);
	if (entry!=NULL) {
		entry->value = value;
	} else {
		int hashCode = this->hashCode(key);
		int index = (hashCode & (this->capacity-1));
		Entry *old = this->table[index];
		Entry *entry = (Entry *)malloc(sizeof(Entry));
		entry->key = key;
		entry->value = value;
		entry->next = old;
		this->table[index] = entry;
	}
    return STATUS_SUCCESS;
}

static void reBuildMap(HashMap *this, Entry **table, Entry *entry, int capacity) {
	if (this!=NULL && table!=NULL && entry!=NULL && capacity>0) {
		int hashCode = this->hashCode(entry->key);
		int index = hashCode & (capacity-1);
		entry->next = table[index];
		table[index] = entry;			
	}
}

static Entry* getEntryByKey(HashMap *this, Object *key) {
	if (this!=NULL && key!=NULL) {
		int hashCode = this->hashCode(key);
		int index = (hashCode & (this->capacity-1));
		Entry *entry = this->table[index];
		while(entry!=NULL) {
			if (this->equalFunc(entry->key, key)) {
				return entry;			
			}
			entry = entry->next;		
		}
	}
	return NULL;
}

void releaseHashMap(Map* this) {
	if (this != NULL) {
		HashMap *hashMap = (HashMap*)this;
		int capacity = hashMap->capacity;
		int i=0;
		for (i=0;i<capacity;++i) {
			Entry *entry = hashMap->table[i];			
			Entry *nextEntry = NULL;
			while(entry!=NULL) {
				nextEntry = entry->next;
				free(entry);
				entry = nextEntry;	
			}
			hashMap->table[i] = NULL;	
		}
		free(hashMap->table);
		free(hashMap);			
	}
}

