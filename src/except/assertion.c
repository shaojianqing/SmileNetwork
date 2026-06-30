#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"

#include "exception.h"
#include "assertion.h"

Exception ValidationException = {ValidationExceptionType};
Exception NullPointerException = {NullPointerExceptionType};
Exception DataIllegalException = {DataIllegalExceptionType};

void assertTrueReal(bool value, char *message, char *filename, int line) {
    if (!value) {
        Exception *exception = &ValidationException;
        throwException(exception, message, filename, line);
    }
}

void assertNotNullReal(Object *ptr, char *message, char *filename, int line) {
    if (ptr == NULL) {
        Exception *exception = &NullPointerException;
        throwException(exception, message, filename, line);
    }
}

void assertDataMatchReal(int value1, int value2, char *message, char *filename, int line) {
    if (value1 != value2) {
        Exception *exception = &DataIllegalException;
        throwException(exception, message, filename, line);
    }
}