#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "stringtype.h"

#define DEFALUT_STRING_SPACE		16
#define DEFAULT_BUFFER_SIZE     	2048

static void bindFunction(String *this);

static int getLength(String *this);

static char* getValue(String *this);

static String* subString(String *this, int start, int end);

static String* catString(String *this, String *string); 

String* createString(char *value) {
	if (value == NULL) {
		return NULL;
	}

	int length = strlen(value);
	if (length == 0) {
		return NULL;
	}

	String *string = (String *)malloc(sizeof(String));
	if (string == NULL) {
		return NULL;
	}

	bindFunction(string);
	string->length = length;
	string->value = malloc(length + 1);
	memcpy(string->value, value, length);
	string->value[length] = '\0';

	return string;
}

String* createStringWithFormat(const char *format, ...) {
	char buffer[DEFAULT_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

	return createString(buffer);
}

void releaseString(String* string) {
	if (string != NULL) {
		if (string->value != NULL) {
			free(string->value);
		}
		free(string);
	}
}

static void bindFunction(String *this) {
	if (this!=NULL) {
		this->getLength = getLength;
		this->getValue = getValue;
		this->subString = subString;
		this->catString = catString;	
	}
}

static int getLength(String *this) {
	if (this != NULL) {
		return this->length;
	} else {
		return 0;	
	}
}

static char* getValue(String *this) {
	if (this!=NULL) {
		return this->value;
	} else {
		return NULL;	
	}
}

static String* subString(String *this, int start, int end) {
	if (this == NULL) {
		return NULL;
	}

	if (start<=end && start>=0 && end<this->length) {
		String *string = (String *)malloc(sizeof(String));
		if (string != NULL) {
			bindFunction(string);

			int length = end - start;
			char *value = malloc(length + 1);
			memcpy(value, this->value + start, length);
			value[string->length] = '\0';

			string->value = value;
			string->length = length;
			return string;
		}
	}
	return NULL;
}

static String* catString(String *this, String *string) {
	if (this != NULL) {
		if (string != NULL && string->length > 0) {
			String *newString = (String *)malloc(sizeof(String));
			if (newString != NULL) {
				bindFunction(newString);

				int totalLength = this->length + string->length;
				char *newValue = malloc(totalLength + 1);
				memcpy(newValue, this->value, this->length);
				memcpy(newValue + this->length, string->value, string->length);
				newValue[totalLength] = '\0';

				newString->value = newValue;
				newString->length = totalLength;
				return newString;
			}
		}
	}
	return NULL;
}

int StringHashCode(Object *object) {
	String *string = (String*)object;
	char *value = string->getValue(string);
	if (value != NULL && *value != '\0') {
		int hashCode = 0;
		while (*value!='\0') {
			hashCode = hashCode*31 + (*value);
			value++;
		}	
		return hashCode;
	} else {
		return 0;	
	}
}

bool StringEqualFun(Object *object1, Object *object2) {
	String *string1 = (String*)object1;
	String *string2 = (String*)object2;
	if (string1!=NULL && string2!=NULL) {
		if (string1==string2) {
			return true;		
		} else {
			char *str1 = string1->getValue(string1);
			char *str2 = string2->getValue(string2);
			return (strcmp(str1, str2)==0);
		}
	} else if (string1==NULL && string2==NULL) {
		return true;
	} else {
		return false;	
	}
}