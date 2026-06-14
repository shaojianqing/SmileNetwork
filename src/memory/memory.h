
#define allocate(size) (alloc(__FILE__, __LINE__, size))

Object* alloc(char *file, int line, int size);

void release(Object* ptr);