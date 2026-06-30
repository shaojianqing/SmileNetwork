#define STATUS_SUCCESS	1
#define STATUS_FAILURE  0

typedef struct Map Map;

struct Map {

	int (*put)(Map *this, Object *key, Object *value);

	Object* (*get)(Map *this, Object *key);

	bool (*containsKey)(Map *this, Object *key);

	int (*remove)(Map *this, Object *key);

	int (*getCount)(Map *this);
};

Map* createHashMap(HashCode hashCode, EqualFunc equalFunc, int capacity);

void releaseHashMap(Map* this);

int StringHashCode(Object *string);

bool StringEqualFun(Object *obj1, Object *obj2);
