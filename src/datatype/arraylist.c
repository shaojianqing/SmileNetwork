#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "datatype.h"
#include "arraylist.h"

typedef struct ArrayList {

	List list;
	
	Object **dataList;

	int capacity;

	int size;

	EqualFunc equal;

} ArrayList;

static int getSize(List *this);

static bool add(List *this, Object *object);

static Object* get(List *this, int index);

static bool removeByObject(List *this, Object *object);

static bool containsObject(List *this, Object *object);

static bool expandArrayList(ArrayList *this);

static bool addInner(ArrayList *this, Object *object);

List *createArrayList(EqualFunc equal, int capacity) {
	if (equal != NULL && capacity > 0) {
		ArrayList *arrayList = (ArrayList *)malloc(sizeof(ArrayList));
		if (arrayList!=NULL) {
			arrayList->dataList = malloc(sizeof(Object *)*capacity);
			memset(arrayList->dataList, 0, sizeof(Object *)*capacity);
			arrayList->equal = equal;
			arrayList->capacity = capacity;
            arrayList->size = 0;
			
			List *list = (List*)arrayList;

			list->add = add;
			list->get = get;			
			list->getSize = getSize;
			list->remove = removeByObject;
			list->containsObject = containsObject;
		}
		return (List *)arrayList;
	} else {
		return NULL;
	}
}

void releaseArrayList(List *this) {
	if (this != NULL) {
		ArrayList *arrayList = (ArrayList *)this;
		free(arrayList->dataList);
		free(this);
	}
}

static int getSize(List *this) {
	if (this != NULL) {
		ArrayList *arrayList = (ArrayList *)this;
		return arrayList->size;	
	}
	return 0;
}

static bool add(List *this, Object *object) {
	if (this != NULL && object != NULL) {
		ArrayList *arrayList = (ArrayList *)this;
		if (arrayList->size >= arrayList->capacity) {
			expandArrayList(arrayList);
		}
		addInner(arrayList, object);
		return true;
	}
	return false;
}

static Object* get(List *this, int index) {
	if (this != NULL) {
		ArrayList *arrayList = (ArrayList *)this;
		if (index >= 0 && index < arrayList->size) {
			return arrayList->dataList[index];	
		}
	}
	return NULL;
}

static bool removeByObject(List *this, Object *object) {
	if (this != NULL && object != NULL) {
		ArrayList *arrayList = (ArrayList*)this;
		int i=0, j=0;
		while (i<arrayList->size) {
			if (arrayList->equal(arrayList->dataList[i], object)) {
				break;
			}
			i++;		
		}
		j=i;
		while(j<arrayList->size-1) {
			arrayList->dataList[j]=arrayList->dataList[j+1];
			j++;		
		}	
		arrayList->dataList[arrayList->size-1] = NULL;
		arrayList->size--;
		return true;
	}
	return false;
}

static bool containsObject(List *this, Object *object) {
	if (this != NULL && object != NULL) {
		ArrayList *arrayList = (ArrayList*)this;
		for (int i=0;i<arrayList->size;++i) {
			if (arrayList->equal(arrayList->dataList[i], object)) {
				return true;
			}	
		}
	}
	return false;
}

static bool addInner(ArrayList *this, Object *object) {
	int index = this->size;
	this->dataList[index] = object;
	this->size++;
    return true;
}

static bool expandArrayList(ArrayList *this) {
	int newCapacity = this->capacity*2;
	Object **oldList = this->dataList;
	Object **newList = malloc(sizeof(Object *)*newCapacity);
	if (newList!=NULL) {
		memset(newList, 0, sizeof(Object *)*newCapacity);
		int i=0;
		for(i=0;i<this->size;++i) {
			newList[i] = oldList[i];		
		}	
		this->dataList = newList;
		this->capacity = newCapacity;
		free(oldList);
		return true;
	}
	return false;
}


