
typedef struct ArrayList ArrayList;

struct ArrayList {
	
	Object **list;

	int capacity;

	int size;

	int (*getSize)(ArrayList *this);

	bool (*add)(ArrayList *this, Object *object);

	Object* (*get)(ArrayList *this, int index);

	bool (*remove)(ArrayList *this, Object *object);
	
	bool (*containsObject)(ArrayList *this, Object *object);

	EqualFunc equalFunc;
};

ArrayList *createArrayList(EqualFunc equalFunc, int capacity);

void releaseArrayList(ArrayList *this);

