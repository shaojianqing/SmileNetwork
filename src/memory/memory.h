
#define allocate(size) (alloc(__FILE__, __LINE__, size))


void initMemoryConfig();

void printAllocationTable();

Object* alloc(char *file, int line, int size);

void release(Object* ptr);