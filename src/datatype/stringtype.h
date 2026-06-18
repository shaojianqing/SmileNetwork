typedef struct String String;

struct String {
	
	long length;

	char *value;

    long (*getLength)(String *this);

	char* (*getValue)(String *this);

	String* (*subString)(String *this, int startIndex, int endIndex);

	String* (*catString)(String *this, String *string);
};

String* createString(char *value);

String* createStringWithFormat(const char *format, ...);

void releaseString(String* string);

int StringHashCode(Object *string);

bool StringEqualFun(Object *string1, Object *string2);
