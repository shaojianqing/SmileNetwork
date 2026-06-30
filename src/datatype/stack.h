typedef struct Stack Stack;

struct Stack {

    bool (*push)(Stack *this, Object *object);

    Object* (*pop)(Stack *this);

    Object* (*peek)(Stack *this);

    int (*getStackCount)(Stack *this);
};

Stack* createStack(int initCount);

void releaseStack(Stack *stack);
