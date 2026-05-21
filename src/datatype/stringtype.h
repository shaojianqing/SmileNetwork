typedef struct String String;

struct String {
	
	int length;

	char *value;

	int (*getLength)(String *this);

	char* (*getValue)(String *this);

	String* (*subString)(String *this, int start, int end);

	String* (*catString)(String *this, String *string);
};

String* createString(char *value);

int StringHashCode(Object *string);

bool StringEqualFun(Object *string1, Object *string2);