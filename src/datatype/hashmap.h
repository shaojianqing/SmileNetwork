
#define LIMIT_FACTOR 0.75

#define STATUS_SUCCESS	1
#define STATUS_FAILURE  0

typedef struct Entry Entry;

typedef struct HashMap HashMap;

struct Entry {
	
	Object *key;
	
	Object *value;

	Entry *next;	
};

struct HashMap {
	
	Entry **table;

	int count;

	int limit;

	int capacity;

	int (*put)(HashMap *this, Object *key, Object *value);

	Object* (*get)(HashMap *this, Object *key);

	int (*getCount)(HashMap *this);

	int (*remove)(HashMap *this, Object *key);

	bool (*containsKey)(HashMap *this, Object *key);

	HashCode hashCode;

	EqualFunc equalFunc;
};

HashMap* createHashMap(HashCode hashCode, EqualFunc equalFunc, int capacity);

void releaseHashMap(HashMap* this);

int StringHashCode(Object *string);

bool StringEqualFun(Object *obj1, Object *obj2);
