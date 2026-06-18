typedef struct Stack Stack;

Stack* createStack(int initCount);

bool push(Stack *this, Object *object);

Object* pop(Stack *this);

Object* peek(Stack *this);

int getStackCount(Stack *this);