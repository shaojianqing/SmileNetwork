
#define assertNotNull(p, m) assertNotNullReal(p,  m, __FILE__, __LINE__)

#define assertDataMatch(u, v, m) assertDataMatchReal(u, v,  m, __FILE__, __LINE__)

void assertNotNullReal(Object *ptr, char *message, char *filename, int line);

void assertDataMatchReal(int value1, int value2, char *message, char *filename, int line);
