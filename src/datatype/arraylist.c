#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "datatype.h"
#include "arraylist.h"

static int getSize(ArrayList *this);

static bool add(ArrayList *this, Object *object);

static Object* get(ArrayList *this, int index);

static bool removeByObject(ArrayList *this, Object *object);

static bool containsObject(ArrayList *this, Object *object);

static bool expandArrayList(ArrayList *this);

static bool addInner(ArrayList *this, Object *object);

ArrayList *createArrayList(EqualFunc equalFunc, int capacity) {
	if (equalFunc!=NULL && capacity>0) {
		ArrayList *arrayList = (ArrayList *)malloc(sizeof(ArrayList));
		if (arrayList!=NULL) {
			arrayList->list = malloc(sizeof(Object *)*capacity);
			memset(arrayList->list, 0, sizeof(Object *)*capacity);
            arrayList->size = 0;

			arrayList->capacity = capacity;
			arrayList->add = add;
			arrayList->get = get;			
			arrayList->getSize = getSize;
			arrayList->equalFunc = equalFunc;
			arrayList->remove = removeByObject;
			arrayList->containsObject = containsObject;
		}
		return arrayList;
	} else {
		return NULL;
	}
}

void releaseArrayList(ArrayList *this) {
	if (this!=NULL) {
		free(this->list);
		free(this);
	}
}

static int getSize(ArrayList *this) {
	if (this!=NULL) {
		return this->size;	
	} else {
		return 0;
	}
}

static bool add(ArrayList *this, Object *object) {
	if (this!=NULL && object!=NULL) {
		if (this->size>=this->capacity) {
			expandArrayList(this);
		}
		addInner(this, object);
		return true;
	}
	return false;
}

static Object* get(ArrayList *this, int index) {
	if (this!=NULL && index>=0 && index<this->size) {
		return this->list[index];	
	}
	return NULL;
}

static bool removeByObject(ArrayList *this, Object *object) {
	if (this!=NULL && object!=NULL) {
		int i=0, j=0;
		while (i<this->size) {
			if (this->equalFunc(this->list[i], object)) {
				break;
			}
			i++;		
		}
		j=i;
		while(j<this->size-1) {
			this->list[j]=this->list[j+1];
			j++;		
		}	
		this->list[this->size-1] = NULL;
		this->size--;
		return true;
	}
	return false;
}

static bool containsObject(ArrayList *this, Object *object) {
	if (this!=NULL && object!=NULL) {
		int i=0;
		while (i<this->size) {
			if (this->equalFunc(this->list[i], object)) {
				return true;
			}		
		}
	}
	return false;
}

static bool addInner(ArrayList *this, Object *object) {
	int index = this->size;
	this->list[index] = object;
	this->size++;
    return true;
}

static bool expandArrayList(ArrayList *this) {
	int newCapacity = this->capacity*2;
	Object **oldList = this->list;
	Object **newList = malloc(sizeof(Object *)*newCapacity);
	if (newList!=NULL) {
		memset(newList, 0, sizeof(Object *)*newCapacity);
		int i=0;
		for(i=0;i<this->size;++i) {
			newList[i] = oldList[i];		
		}	
		this->list = newList;
		this->capacity = newCapacity;
		free(oldList);
		return true;
	}
	return false;
}


