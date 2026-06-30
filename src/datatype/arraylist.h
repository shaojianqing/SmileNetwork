
typedef struct List List;

struct List {

	bool (*add)(List *this, Object *object);

	Object* (*get)(List *this, int index);

	bool (*remove)(List *this, Object *object);
	
	bool (*containsObject)(List *this, Object *object);

	int (*getSize)(List *this);
};

List *createArrayList(EqualFunc equalFunc, int capacity);

void releaseArrayList(List *this);

