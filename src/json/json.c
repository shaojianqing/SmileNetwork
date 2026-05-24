#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#include "json.h"

/* define our own boolean type */
#define true ((JsonBool)1)
#define false ((JsonBool)0)

/* define isnan and isinf for ANSI C, if in C99 or above, isnan and isinf has been defined in math.h */
#ifndef isinf
#define isinf(d) (isnan((d - d)) && !isnan(d))
#endif
#ifndef isnan
#define isnan(d) (d != d)
#endif

#ifndef NAN
#ifdef _WIN32
#define NAN sqrt(-1.0)
#else
#define NAN 0.0/0.0
#endif
#endif

typedef struct Error Error;

/* The Json structure: */
struct Json {
    /* next/prev allow you to walk array/object chains. Alternatively, use GetArraySize/GetArrayItem/GetObjectItem */
    Json *next;
    Json *prev;
    /* An array or object item will have a child pointer pointing to a chain of the items in the array/object. */
    Json *child;
    /* The type of the item, as above. */
    int type;
    /* writing to valueInt is DEPRECATED, use setJsonNumberValue instead */
    int valueInt;
    /* The item's number, if type==JsonNumber */
    double valueDouble;
    /* The item's string, if type==JsonString  and type == JsonRaw */
    char *valueString;
    /* The item's name string, if this item is the child of, or is in the list of subitems of an object. */
    char *string;
};

struct Error {
    const unsigned char *json;
    size_t position;
};

static Error globalError = { NULL, 0 };

const char* getJsonErrorPtr(void) {
    return (const char*) (globalError.json + globalError.position);
}

char* getJsonStringValue(const Json *item) {
    if (!isJsonString(item)) {
        return NULL;
    }
    return item->valueString;
}

double getJsonNumberValue(const Json *item) {
    if (!isJsonNumber(item)) {
        return (double) NAN;
    }
    return item->valueDouble;
}

/* Case insensitive string comparison, doesn't consider two NULL pointers equal though */
static int caseInsensitiveStrcmp(const unsigned char *string1, const unsigned char *string2) {
    if ((string1 == NULL) || (string2 == NULL)) {
        return 1;
    }

    if (string1 == string2) {
        return 0;
    }

    for(; tolower(*string1) == tolower(*string2); (void)string1++, string2++) {
        if (*string1 == '\0') {
            return 0;
        }
    }
    return tolower(*string1) - tolower(*string2);
}


/* strlen of character literals resolved at compile time */
#define staticStrlen(stringLiteral) (sizeof(stringLiteral) - sizeof(""))

static unsigned char* Jsonstrdup(const unsigned char* string) {
    size_t length = 0;
    unsigned char *copy = NULL;

    if (string == NULL) {
        return NULL;
    }

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*)malloc(length);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, string, length);
    return copy;
}

/* Internal constructor. */
static Json *newJsonItem() {
    Json* node = (Json*)malloc(sizeof(Json));
    if (node) {
        memset(node, '\0', sizeof(Json));
    }

    return node;
}

/* Delete a Json structure. */
void deleteJson(Json *item) {
    Json *next = NULL;
    while (item != NULL) {
        next = item->next;
        if (!(item->type & IsJsonReference) && (item->child != NULL)) {
            deleteJson(item->child);
        }
        if (!(item->type & IsJsonReference) && (item->valueString != NULL)) {
            free(item->valueString);
            item->valueString = NULL;
        }
        if (!(item->type & IsJsonStringConst) && (item->string != NULL)) {
            free(item->string);
            item->string = NULL;
        }
        free(item);
        item = next;
    }
}

/* get the decimal point character of the current locale */
static unsigned char getDecimalPoint(void) {
    return '.';
}

typedef struct {
    const unsigned char *content;
    size_t length;
    size_t offset;
    size_t depth; /* How deeply nested (in arrays/objects) is the input at the current offset. */
} ParseBuffer;

/* check if the given size is left to read in a given parse buffer (starting with 1) */
#define canRead(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
/* check if the buffer can be accessed at the given index (starting with 0) */
#define canAccessAtIndex(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannotAccessAtIndex(buffer, index) (!canAccessAtIndex(buffer, index))
/* get a pointer to the buffer at the position */
#define bufferAtOffset(buffer) ((buffer)->content + (buffer)->offset)

/* Parse the input text to generate a number, and populate the result into item. */
static JsonBool parseNumber(Json * const item, ParseBuffer * const inputBuffer) {
    double number = 0;
    unsigned char *afterEnd = NULL;
    unsigned char *numberCString;
    unsigned char decimalPoint = getDecimalPoint();
    size_t i = 0;
    size_t numberStringLength = 0;
    JsonBool hasDecimalPoint = false;

    if ((inputBuffer == NULL) || (inputBuffer->content == NULL)) {
        return false;
    }

    /* copy the number into a temporary buffer and replace '.' with the decimal point
     * of the current locale (for strtod)
     * This also takes care of '\0' not necessarily being available for marking the end of the input */
    for (i = 0; canAccessAtIndex(inputBuffer, i); i++) {
        switch (bufferAtOffset(inputBuffer)[i]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '+':
            case '-':
            case 'e':
            case 'E':
                numberStringLength++;
                break;

            case '.':
                numberStringLength++;
                hasDecimalPoint = true;
                break;

            default:
                goto loopEnd;
        }
    }
loopEnd:
    /* malloc for temporary buffer, add 1 for '\0' */
    numberCString = (unsigned char *)malloc(numberStringLength + 1);
    if (numberCString == NULL) {
        return false; /* allocation failure */
    }

    memcpy(numberCString, bufferAtOffset(inputBuffer), numberStringLength);
    numberCString[numberStringLength] = '\0';

    if (hasDecimalPoint) {
        for (i = 0; i < numberStringLength; i++) {
            if (numberCString[i] == '.') {
                /* replace '.' with the decimal point of the current locale (for strtod) */
                numberCString[i] = decimalPoint;
            }
        }
    }

    number = strtod((const char*)numberCString, (char**)&afterEnd);
    if (numberCString == afterEnd) {
        /* free the temporary buffer */
        free(numberCString);
        return false; /* parseError */
    }

    item->valueDouble = number;

    /* use saturation in case of overflow */
    if (number >= INT_MAX) {
        item->valueInt = INT_MAX;
    } else if (number <= (double)INT_MIN) {
        item->valueInt = INT_MIN;
    } else {
        item->valueInt = (int)number;
    }

    item->type = JsonNumber;

    inputBuffer->offset += (size_t)(afterEnd - numberCString);
    /* free the temporary buffer */
    free(numberCString);
    return true;
}

/* don't ask me, but the original setJsonNumberValue returns an integer or double */
double setJsonNumberHelper(Json *object, double number) {
    if (object == NULL) {
        return (double)NAN;
    }

    if (number >= INT_MAX)  {
        object->valueInt = INT_MAX;
    } else if (number <= (double)INT_MIN) {
        object->valueInt = INT_MIN;
    } else {
        object->valueInt = (int)number;
    }

    return object->valueDouble = number;
}

/* Note: when passing a NULL valueString, valueString treats this as an error and return NULL */
char* setJsonValueString(Json *object, const char *valueString) {
    char *copy = NULL;
    size_t v1Len;
    size_t v2Len;
    /* if object's type is not JsonString or is IsJsonReference, it should not set valueString */
    if ((object == NULL) || !(object->type & JsonString) || (object->type & IsJsonReference)) {
        return NULL;
    }
    /* return NULL if the object is corrupted or valueString is NULL */
    if (object->valueString == NULL || valueString == NULL) {
        return NULL;
    }

    v1Len = strlen(valueString);
    v2Len = strlen(object->valueString);

    if (v1Len <= v2Len) {
        /* strcpy does not handle overlapping string: [X1, X2] [Y1, Y2] => X2 < Y1 or Y2 < X1 */
        if (!( valueString + v1Len < object->valueString || object->valueString + v2Len < valueString )) {
            return NULL;
        }
        strcpy(object->valueString, valueString);
        return object->valueString;
    }
    copy = (char*) Jsonstrdup((const unsigned char*)valueString);
    if (copy == NULL) {
        return NULL;
    }
    if (object->valueString != NULL) {
        free(object->valueString);
        object->valueString = NULL;
    }
    object->valueString = copy;

    return copy;
}

typedef struct {
    unsigned char *buffer;
    size_t length;
    size_t offset;
    size_t depth; /* current nesting depth (for formatted printing) */
    JsonBool noalloc;
    JsonBool format; /* is this print a formatted print */
} PrintBuffer;

/* realloc PrintBuffer if necessary to have at least "needed" bytes more */
static unsigned char* ensure(PrintBuffer * const p, size_t needed) {
    unsigned char *newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL)) {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length)) {
        /* make sure that offset is valid */
        return NULL;
    }

    if (needed > INT_MAX) {
        /* sizes bigger than INT_MAX are currently not supported */
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length) {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    /* calculate new buffer size */
    if (needed > (INT_MAX / 2)) {
        /* overflow of int, use INT_MAX if possible */
        if (needed <= INT_MAX) {
            newsize = INT_MAX;
        } else {
            return NULL;
        }
    } else {
        newsize = needed * 2;
    }

    newbuffer = (unsigned char*)realloc(p->buffer, newsize);
    if (newbuffer == NULL) {
        free(p->buffer);
        p->length = 0;
        p->buffer = NULL;

        return NULL;
    }

    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

/* calculate the new length of the string in a PrintBuffer and update the offset */
static void updateOffset(PrintBuffer * const buffer) {
    const unsigned char *bufferPointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL)) {
        return;
    }
    bufferPointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char*)bufferPointer);
}

/* securely comparison of floating-point variables */
static JsonBool compareDouble(double a, double b) {
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

/* Render the number nicely from the given item into a string. */
static JsonBool printNumber(const Json * const item, PrintBuffer * const outputBuffer) {
    unsigned char *outputPointer = NULL;
    double d = item->valueDouble;
    int length = 0;
    size_t i = 0;
    unsigned char numberBuffer[26] = {0}; /* temporary buffer to print the number into */
    unsigned char decimalPoint = getDecimalPoint();
    double test = 0.0;

    if (outputBuffer == NULL) {
        return false;
    }

    /* This checks for NaN and Infinity */
    if (isnan(d) || isinf(d)) {
        length = sprintf((char*)numberBuffer, "null");
    } else if(d == (double)item->valueInt) {
        length = sprintf((char*)numberBuffer, "%d", item->valueInt);
    } else {
        /* Try 15 decimal places of precision to avoid nonsignificant nonzero digits */
        length = sprintf((char*)numberBuffer, "%1.15g", d);

        /* Check whether the original double can be recovered */
        if ((sscanf((char*)numberBuffer, "%lg", &test) != 1) || !compareDouble((double)test, d)) {
            /* If not, print with 17 decimal places of precision */
            length = sprintf((char*)numberBuffer, "%1.17g", d);
        }
    }

    /* sprintf failed or buffer overrun occurred */
    if ((length < 0) || (length > (int)(sizeof(numberBuffer) - 1))) {
        return false;
    }

    /* reserve appropriate space in the output */
    outputPointer = ensure(outputBuffer, (size_t)length + sizeof(""));
    if (outputPointer == NULL) {
        return false;
    }

    /* copy the printed number to the output and replace locale
     * dependent decimal point with '.' */
    for (i = 0; i < ((size_t)length); i++) {
        if (numberBuffer[i] == decimalPoint) {
            outputPointer[i] = '.';
            continue;
        }

        outputPointer[i] = numberBuffer[i];
    }
    outputPointer[i] = '\0';
    outputBuffer->offset += (size_t)length;

    return true;
}

/* parse 4 digit hexadecimal number */
static unsigned parseHex4(const unsigned char * const input) {
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++) {
        /* parse digit */
        if ((input[i] >= '0') && (input[i] <= '9')) {
            h += (unsigned int) input[i] - '0';
        } else if ((input[i] >= 'A') && (input[i] <= 'F')) {
            h += (unsigned int) 10 + input[i] - 'A';
        }  else if ((input[i] >= 'a') && (input[i] <= 'f')) {
            h += (unsigned int) 10 + input[i] - 'a';
        } else /* invalid */ {
            return 0;
        }

        if (i < 3) {
            /* shift left to make place for the next nibble */
            h = h << 4;
        }
    }
    return h;
}

/* converts a UTF-16 literal to UTF-8
 * A literal can be one or two sequences of the form \uXXXX */
static unsigned char utf16LiteralToUtf8(const unsigned char * const inputPointer, const unsigned char * const inputEnd, unsigned char **outputPointer) {
    long unsigned int codepoint = 0;
    unsigned int firstCode = 0;
    const unsigned char *firstSequence = inputPointer;
    unsigned char utf8Length = 0;
    unsigned char utf8Position = 0;
    unsigned char sequenceLength = 0;
    unsigned char firstByteMark = 0;

    if ((inputEnd - firstSequence) < 6) {
        /* input ends unexpectedly */
        goto fail;
    }

    /* get the first utf16 sequence */
    firstCode = parseHex4(firstSequence + 2);

    /* check that the code is valid */
    if (((firstCode >= 0xDC00) && (firstCode <= 0xDFFF))) {
        goto fail;
    }

    /* UTF16 surrogate pair */
    if ((firstCode >= 0xD800) && (firstCode <= 0xDBFF)) {
        const unsigned char *secondSequence = firstSequence + 6;
        unsigned int secondCode = 0;
        sequenceLength = 12; /* \uXXXX\uXXXX */

        if ((inputEnd - secondSequence) < 6) {
            /* input ends unexpectedly */
            goto fail;
        }

        if ((secondSequence[0] != '\\') || (secondSequence[1] != 'u')) {
            /* missing second half of the surrogate pair */
            goto fail;
        }

        /* get the second utf16 sequence */
        secondCode = parseHex4(secondSequence + 2);
        /* check that the code is valid */
        if ((secondCode < 0xDC00) || (secondCode > 0xDFFF)) {
            /* invalid second half of the surrogate pair */
            goto fail;
        }


        /* calculate the unicode codepoint from the surrogate pair */
        codepoint = 0x10000 + (((firstCode & 0x3FF) << 10) | (secondCode & 0x3FF));
    } else {
        sequenceLength = 6; /* \uXXXX */
        codepoint = firstCode;
    }

    /* encode as UTF-8
     * takes at maximum 4 bytes to encode:
     * 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
    if (codepoint < 0x80) {
        /* normal ascii, encoding 0xxxxxxx */
        utf8Length = 1;
    } else if (codepoint < 0x800) {
        /* two bytes, encoding 110xxxxx 10xxxxxx */
        utf8Length = 2;
        firstByteMark = 0xC0; /* 11000000 */
    } else if (codepoint < 0x10000) {
        /* three bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx */
        utf8Length = 3;
        firstByteMark = 0xE0; /* 11100000 */
    } else if (codepoint <= 0x10FFFF) {
        /* four bytes, encoding 1110xxxx 10xxxxxx 10xxxxxx 10xxxxxx */
        utf8Length = 4;
        firstByteMark = 0xF0; /* 11110000 */
    } else {
        /* invalid unicode codepoint */
        goto fail;
    }

    /* encode as utf8 */
    for (utf8Position = (unsigned char)(utf8Length - 1); utf8Position > 0; utf8Position--) {
        /* 10xxxxxx */
        (*outputPointer)[utf8Position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    /* encode first byte */
    if (utf8Length > 1) {
        (*outputPointer)[0] = (unsigned char)((codepoint | firstByteMark) & 0xFF);
    } else {
        (*outputPointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *outputPointer += utf8Length;
    return sequenceLength;

fail:
    return 0;
}

/* Parse the input text into an unescaped cinput, and populate item. */
static JsonBool parseString(Json * const item, ParseBuffer * const inputBuffer) {
    const unsigned char *inputPointer = bufferAtOffset(inputBuffer) + 1;
    const unsigned char *inputEnd = bufferAtOffset(inputBuffer) + 1;
    unsigned char *outputPointer = NULL;
    unsigned char *output = NULL;

    /* not a string */
    if (bufferAtOffset(inputBuffer)[0] != '\"'){
        goto fail;
    }

    {
        /* calculate approximate size of the output (overestimate) */
        size_t allocationLength = 0;
        size_t skippedBytes = 0;
        while (((size_t)(inputEnd - inputBuffer->content) < inputBuffer->length) && (*inputEnd != '\"')) {
            /* is escape sequence */
            if (inputEnd[0] == '\\') {
                if ((size_t)(inputEnd + 1 - inputBuffer->content) >= inputBuffer->length) {
                    /* prevent buffer overflow when last input character is a backslash */
                    goto fail;
                }
                skippedBytes++;
                inputEnd++;
            }
            inputEnd++;
        }
        if (((size_t)(inputEnd - inputBuffer->content) >= inputBuffer->length) || (*inputEnd != '\"')) {
            goto fail; /* string ended unexpectedly */
        }

        /* This is at most how much we need for the output */
        allocationLength = (size_t) (inputEnd - bufferAtOffset(inputBuffer)) - skippedBytes;
        output = (unsigned char*)malloc(allocationLength + sizeof(""));
        if (output == NULL) {
            goto fail; /* allocation failure */
        }
    }

    outputPointer = output;
    /* loop through the string literal */
    while (inputPointer < inputEnd) {
        if (*inputPointer != '\\') {
            *outputPointer++ = *inputPointer++;
        } else {
            unsigned char sequenceLength = 2;
            if ((inputEnd - inputPointer) < 1) {
                goto fail;
            }

            switch (inputPointer[1]) {
                case 'b':
                    *outputPointer++ = '\b';
                    break;
                case 'f':
                    *outputPointer++ = '\f';
                    break;
                case 'n':
                    *outputPointer++ = '\n';
                    break;
                case 'r':
                    *outputPointer++ = '\r';
                    break;
                case 't':
                    *outputPointer++ = '\t';
                    break;
                case '\"':
                case '\\':
                case '/':
                    *outputPointer++ = inputPointer[1];
                    break;

                /* UTF-16 literal */
                case 'u':
                    sequenceLength = utf16LiteralToUtf8(inputPointer, inputEnd, &outputPointer);
                    if (sequenceLength == 0)
                    {
                        /* failed to convert UTF16-literal to UTF-8 */
                        goto fail;
                    }
                    break;

                default:
                    goto fail;
            }
            inputPointer += sequenceLength;
        }
    }

    /* zero terminate the output */
    *outputPointer = '\0';

    item->type = JsonString;
    item->valueString = (char*)output;

    inputBuffer->offset = (size_t) (inputEnd - inputBuffer->content);
    inputBuffer->offset++;

    return true;

fail:
    if (output != NULL) {
        free(output);
        output = NULL;
    }

    if (inputPointer != NULL) {
        inputBuffer->offset = (size_t)(inputPointer - inputBuffer->content);
    }

    return false;
}

/* Render the cstring provided to an escaped version that can be printed. */
static JsonBool printStringPtr(const unsigned char * const input, PrintBuffer * const outputBuffer) {
    const unsigned char *inputPointer = NULL;
    unsigned char *output = NULL;
    unsigned char *outputPointer = NULL;
    size_t outputLength = 0;
    /* numbers of additional characters needed for escaping */
    size_t escapeCharacters = 0;

    if (outputBuffer == NULL) {
        return false;
    }

    /* empty string */
    if (input == NULL) {
        output = ensure(outputBuffer, sizeof("\"\""));
        if (output == NULL) {
            return false;
        }
        strcpy((char*)output, "\"\"");

        return true;
    }

    /* set "flag" to 1 if something needs to be escaped */
    for (inputPointer = input; *inputPointer; inputPointer++) {
        switch (*inputPointer) {
            case '\"':
            case '\\':
            case '\b':
            case '\f':
            case '\n':
            case '\r':
            case '\t':
                /* one character escape sequence */
                escapeCharacters++;
                break;
            default:
                if (*inputPointer < 32) {
                    /* UTF-16 escape sequence uXXXX */
                    escapeCharacters += 5;
                }
                break;
        }
    }
    outputLength = (size_t)(inputPointer - input) + escapeCharacters;

    output = ensure(outputBuffer, outputLength + sizeof("\"\""));
    if (output == NULL) {
        return false;
    }

    /* no characters have to be escaped */
    if (escapeCharacters == 0) {
        output[0] = '\"';
        memcpy(output + 1, input, outputLength);
        output[outputLength + 1] = '\"';
        output[outputLength + 2] = '\0';

        return true;
    }

    output[0] = '\"';
    outputPointer = output + 1;
    /* copy the string */
    for (inputPointer = input; *inputPointer != '\0'; (void)inputPointer++, outputPointer++) {
        if ((*inputPointer > 31) && (*inputPointer != '\"') && (*inputPointer != '\\')) {
            /* normal character, copy */
            *outputPointer = *inputPointer;
        } else {
            /* character needs to be escaped */
            *outputPointer++ = '\\';
            switch (*inputPointer) {
                case '\\':
                    *outputPointer = '\\';
                    break;
                case '\"':
                    *outputPointer = '\"';
                    break;
                case '\b':
                    *outputPointer = 'b';
                    break;
                case '\f':
                    *outputPointer = 'f';
                    break;
                case '\n':
                    *outputPointer = 'n';
                    break;
                case '\r':
                    *outputPointer = 'r';
                    break;
                case '\t':
                    *outputPointer = 't';
                    break;
                default:
                    /* escape and print as unicode codepoint */
                    sprintf((char*)outputPointer, "u%04x", *inputPointer);
                    outputPointer += 4;
                    break;
            }
        }
    }
    output[outputLength + 1] = '\"';
    output[outputLength + 2] = '\0';

    return true;
}

/* Invoke printStringPtr (which is useful) on an item. */
static JsonBool printString(const Json * const item, PrintBuffer * const p) {
    return printStringPtr((unsigned char*)item->valueString, p);
}

/* Predeclare these prototypes. */
static JsonBool parseValue(Json * const item, ParseBuffer * const inputBuffer);
static JsonBool printValue(const Json * const item, PrintBuffer * const outputBuffer);
static JsonBool parseArray(Json * const item, ParseBuffer * const inputBuffer);
static JsonBool printArray(const Json * const item, PrintBuffer * const outputBuffer);
static JsonBool parseObject(Json * const item, ParseBuffer * const inputBuffer);
static JsonBool printObject(const Json * const item, PrintBuffer * const outputBuffer);

/* Utility to jump whitespace and cr/lf */
static ParseBuffer *bufferSkipWhitespace(ParseBuffer * const buffer) {
    if ((buffer == NULL) || (buffer->content == NULL)) {
        return NULL;
    }

    if (cannotAccessAtIndex(buffer, 0)) {
        return buffer;
    }

    while (canAccessAtIndex(buffer, 0) && (bufferAtOffset(buffer)[0] <= 32)) {
       buffer->offset++;
    }

    if (buffer->offset == buffer->length) {
        buffer->offset--;
    }

    return buffer;
}

/* skip the UTF-8 BOM (byte order mark) if it is at the beginning of a buffer */
static ParseBuffer *skipUtf8Bom(ParseBuffer * const buffer) {
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0)) {
        return NULL;
    }

    if (canAccessAtIndex(buffer, 4) && (strncmp((const char*)bufferAtOffset(buffer), "\xEF\xBB\xBF", 3) == 0)) {
        buffer->offset += 3;
    }

    return buffer;
}

Json* parseJsonWithOpts(const char *value, const char **returnParseEnd, JsonBool requireNullTerminated) {
    size_t bufferLength;

    if (value == NULL) {
        return NULL;
    }

    /* Adding null character size due to requireNullTerminated. */
    bufferLength = strlen(value) + sizeof("");
    return parseJsonWithLengthOpts(value, bufferLength, returnParseEnd, requireNullTerminated);
}

/* Parse an object - create a new root, and populate. */
Json* parseJsonWithLengthOpts(const char *value, size_t bufferLength, const char **returnParseEnd, JsonBool requireNullTerminated) {
    ParseBuffer buffer = { 0, 0, 0, 0 };
    Json *item = NULL;

    /* reset error position */
    globalError.json = NULL;
    globalError.position = 0;

    if (value == NULL || 0 == bufferLength) {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = bufferLength;
    buffer.offset = 0;

    item = newJsonItem();
    if (item == NULL) /* memory fail */{
        goto fail;
    }

    if (!parseValue(item, bufferSkipWhitespace(skipUtf8Bom(&buffer)))) {
        /* parse failure. ep is set. */
        goto fail;
    }

    /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
    if (requireNullTerminated) {
        bufferSkipWhitespace(&buffer);
        if ((buffer.offset >= buffer.length) || bufferAtOffset(&buffer)[0] != '\0') {
            goto fail;
        }
    }
    if (returnParseEnd) {
        *returnParseEnd = (const char*)bufferAtOffset(&buffer);
    }

    return item;

fail:
    if (item != NULL) {
        deleteJson(item);
    }

    if (value != NULL) {
        Error localError;
        localError.json = (const unsigned char*)value;
        localError.position = 0;

        if (buffer.offset < buffer.length) {
            localError.position = buffer.offset;
        } else if (buffer.length > 0) {
            localError.position = buffer.length - 1;
        }

        if (returnParseEnd != NULL) {
            *returnParseEnd = (const char*)localError.json + localError.position;
        }

        globalError = localError;
    }

    return NULL;
}

/* Default options for parseJson */
Json* parseJson(const char *value) {
    return parseJsonWithOpts(value, 0, 0);
}

Json* parseJsonWithLength(const char *value, size_t bufferLength) {
    return parseJsonWithLengthOpts(value, bufferLength, 0, 0);
}

#define Jsonmin(a, b) (((a) < (b)) ? (a) : (b))

static unsigned char *print(const Json* item, JsonBool format) {
    static const size_t defaultBufferSize = 256;
    PrintBuffer buffer[1];
    unsigned char *printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    /* create buffer */
    buffer->buffer = (unsigned char*) malloc(defaultBufferSize);
    buffer->length = defaultBufferSize;
    buffer->format = format;
    if (buffer->buffer == NULL) {
        goto fail;
    }

    /* print the value */
    if (!printValue(item, buffer)) {
        goto fail;
    }
    updateOffset(buffer);
    printed = (unsigned char*) realloc(buffer->buffer, buffer->offset + 1);
    if (printed == NULL) {
        goto fail;
    }
    buffer->buffer = NULL;
    return printed;

fail:
    if (buffer->buffer != NULL) {
        free(buffer->buffer);
        buffer->buffer = NULL;
    }

    if (printed != NULL) {
        free(printed);
        printed = NULL;
    }

    return NULL;
}

/* Render a Json item/entity/structure to text. */
char* printJson(const Json *item) {
    return (char*)print(item, true);
}

char* printJsonUnformatted(const Json *item) {
    return (char*)print(item, false);
}

char* printJsonBuffered(const Json *item, int prebuffer, JsonBool fmt) {
    PrintBuffer p = { 0, 0, 0, 0, 0, 0 };

    if (prebuffer < 0) {
        return NULL;
    }

    p.buffer = (unsigned char*)malloc((size_t)prebuffer);
    if (!p.buffer) {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;

    if (!printValue(item, &p)) {
        free(p.buffer);
        p.buffer = NULL;
        return NULL;
    }

    return (char*)p.buffer;
}

JsonBool printJsonPreallocated(Json *item, char *buffer, const int length, const JsonBool format) {
    PrintBuffer p = { 0, 0, 0, 0, 0, 0 };

    if ((length < 0) || (buffer == NULL)) {
        return false;
    }

    p.buffer = (unsigned char*)buffer;
    p.length = (size_t)length;
    p.offset = 0;
    p.noalloc = true;
    p.format = format;

    return printValue(item, &p);
}

/* Parser core - when encountering text, process appropriately. */
static JsonBool parseValue(Json * const item, ParseBuffer * const inputBuffer) {
    if ((inputBuffer == NULL) || (inputBuffer->content == NULL)) {
        return false; /* no input */
    }

    /* parse the different types of values */
    /* null */
    if (canRead(inputBuffer, 4) && (strncmp((const char*)bufferAtOffset(inputBuffer), "null", 4) == 0)) {
        item->type = JsonNULL;
        inputBuffer->offset += 4;
        return true;
    }
    /* false */
    if (canRead(inputBuffer, 5) && (strncmp((const char*)bufferAtOffset(inputBuffer), "false", 5) == 0)) {
        item->type = JsonFalse;
        inputBuffer->offset += 5;
        return true;
    }
    /* true */
    if (canRead(inputBuffer, 4) && (strncmp((const char*)bufferAtOffset(inputBuffer), "true", 4) == 0)) {
        item->type = JsonTrue;
        item->valueInt = 1;
        inputBuffer->offset += 4;
        return true;
    }
    /* string */
    if (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == '\"')) {
        return parseString(item, inputBuffer);
    }
    /* number */
    if (canAccessAtIndex(inputBuffer, 0) && ((bufferAtOffset(inputBuffer)[0] == '-') || ((bufferAtOffset(inputBuffer)[0] >= '0') && (bufferAtOffset(inputBuffer)[0] <= '9')))) {
        return parseNumber(item, inputBuffer);
    }
    /* array */
    if (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == '[')) {
        return parseArray(item, inputBuffer);
    }
    /* object */
    if (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == '{')) {
        return parseObject(item, inputBuffer);
    }

    return false;
}

/* Render a value to text. */
static JsonBool printValue(const Json * const item, PrintBuffer * const outputBuffer) {
    unsigned char *output = NULL;

    if ((item == NULL) || (outputBuffer == NULL)) {
        return false;
    }

    switch ((item->type) & 0xFF){
        case JsonNULL:
            output = ensure(outputBuffer, 5);
            if (output == NULL) {
                return false;
            }
            strcpy((char*)output, "null");
            return true;

        case JsonFalse:
            output = ensure(outputBuffer, 6);
            if (output == NULL) {
                return false;
            }
            strcpy((char*)output, "false");
            return true;

        case JsonTrue:
            output = ensure(outputBuffer, 5);
            if (output == NULL) {
                return false;
            }
            strcpy((char*)output, "true");
            return true;

        case JsonNumber:
            return printNumber(item, outputBuffer);

        case JsonRaw: {
            size_t rawLength = 0;
            if (item->valueString == NULL) {
                return false;
            }

            rawLength = strlen(item->valueString) + sizeof("");
            output = ensure(outputBuffer, rawLength);
            if (output == NULL) {
                return false;
            }
            memcpy(output, item->valueString, rawLength);
            return true;
        }

        case JsonString:
            return printString(item, outputBuffer);

        case JsonArray:
            return printArray(item, outputBuffer);

        case JsonObject:
            return printObject(item, outputBuffer);

        default:
            return false;
    }
}

/* Build an array from input text. */
static JsonBool parseArray(Json *item, ParseBuffer *inputBuffer) {
    Json *head = NULL; /* head of the linked list */
    Json *currentItem = NULL;

    if (inputBuffer->depth >= JSON_NESTING_LIMIT) {
        return false; /* to deeply nested */
    }
    inputBuffer->depth++;

    if (bufferAtOffset(inputBuffer)[0] != '[') {
        /* not an array */
        goto fail;
    }

    inputBuffer->offset++;
    bufferSkipWhitespace(inputBuffer);
    if (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == ']')) {
        /* empty array */
        goto success;
    }

    /* check if we skipped to the end of the buffer */
    if (cannotAccessAtIndex(inputBuffer, 0)) {
        inputBuffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    inputBuffer->offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        Json *newItem = newJsonItem();
        if (newItem == NULL) {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            currentItem = head = newItem;
        } else {
            /* add to the end and advance */
            currentItem->next = newItem;
            newItem->prev = currentItem;
            currentItem = newItem;
        }

        /* parse next value */
        inputBuffer->offset++;
        bufferSkipWhitespace(inputBuffer);
        if (!parseValue(currentItem, inputBuffer)) {
            goto fail; /* failed to parse value */
        }
        bufferSkipWhitespace(inputBuffer);
    }
    while (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == ','));

    if (cannotAccessAtIndex(inputBuffer, 0) || bufferAtOffset(inputBuffer)[0] != ']') {
        goto fail; /* expected end of array */
    }

success:
    inputBuffer->depth--;

    if (head != NULL) {
        head->prev = currentItem;
    }

    item->type = JsonArray;
    item->child = head;

    inputBuffer->offset++;

    return true;

fail:
    if (head != NULL) {
        deleteJson(head);
    }

    return false;
}

/* Render an array to text */
static JsonBool printArray(const Json * const item, PrintBuffer * const outputBuffer) {
    unsigned char *outputPointer = NULL;
    size_t length = 0;
    Json *currentElement = item->child;

    if (outputBuffer == NULL) {
        return false;
    }

    if (outputBuffer->depth >= JSON_NESTING_LIMIT) {
        return false; /* nesting is too deep */
    }

    /* Compose the output array. */
    /* opening square bracket */
    outputPointer = ensure(outputBuffer, 1);
    if (outputPointer == NULL) {
        return false;
    }

    *outputPointer = '[';
    outputBuffer->offset++;
    outputBuffer->depth++;

    while (currentElement != NULL) {
        if (!printValue(currentElement, outputBuffer)) {
            return false;
        }
        updateOffset(outputBuffer);
        if (currentElement->next) {
            length = (size_t) (outputBuffer->format ? 2 : 1);
            outputPointer = ensure(outputBuffer, length + 1);
            if (outputPointer == NULL) {
                return false;
            }
            *outputPointer++ = ',';
            if(outputBuffer->format) {
                *outputPointer++ = ' ';
            }
            *outputPointer = '\0';
            outputBuffer->offset += length;
        }
        currentElement = currentElement->next;
    }

    outputPointer = ensure(outputBuffer, 2);
    if (outputPointer == NULL) {
        return false;
    }
    *outputPointer++ = ']';
    *outputPointer = '\0';
    outputBuffer->depth--;

    return true;
}

/* Build an object from the text. */
static JsonBool parseObject(Json * const item, ParseBuffer * const inputBuffer) {
    Json *head = NULL; /* linked list head */
    Json *currentItem = NULL;

    if (inputBuffer->depth >= JSON_NESTING_LIMIT) {
        return false; /* to deeply nested */
    }
    inputBuffer->depth++;

    if (cannotAccessAtIndex(inputBuffer, 0) || (bufferAtOffset(inputBuffer)[0] != '{')) {
        goto fail; /* not an object */
    }

    inputBuffer->offset++;
    bufferSkipWhitespace(inputBuffer);
    if (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == '}')) {
        goto success; /* empty object */
    }

    /* check if we skipped to the end of the buffer */
    if (cannotAccessAtIndex(inputBuffer, 0)) {
        inputBuffer->offset--;
        goto fail;
    }

    /* step back to character in front of the first element */
    inputBuffer->offset--;
    /* loop through the comma separated array elements */
    do {
        /* allocate next item */
        Json *newItem = newJsonItem();
        if (newItem == NULL) {
            goto fail; /* allocation failure */
        }

        /* attach next item to list */
        if (head == NULL) {
            /* start the linked list */
            currentItem = head = newItem;
        } else {
            /* add to the end and advance */
            currentItem->next = newItem;
            newItem->prev = currentItem;
            currentItem = newItem;
        }

        if (cannotAccessAtIndex(inputBuffer, 1)) {
            goto fail; /* nothing comes after the comma */
        }

        /* parse the name of the child */
        inputBuffer->offset++;
        bufferSkipWhitespace(inputBuffer);
        if (!parseString(currentItem, inputBuffer)) {
            goto fail; /* failed to parse name */
        }
        bufferSkipWhitespace(inputBuffer);

        /* swap valueString and string, because we parsed the name */
        currentItem->string = currentItem->valueString;
        currentItem->valueString = NULL;

        if (cannotAccessAtIndex(inputBuffer, 0) || (bufferAtOffset(inputBuffer)[0] != ':'))
        {
            goto fail; /* invalid object */
        }

        /* parse the value */
        inputBuffer->offset++;
        bufferSkipWhitespace(inputBuffer);
        if (!parseValue(currentItem, inputBuffer)) {
            goto fail; /* failed to parse value */
        }
        bufferSkipWhitespace(inputBuffer);
    }
    while (canAccessAtIndex(inputBuffer, 0) && (bufferAtOffset(inputBuffer)[0] == ','));

    if (cannotAccessAtIndex(inputBuffer, 0) || (bufferAtOffset(inputBuffer)[0] != '}')) {
        goto fail; /* expected end of object */
    }

success:
    inputBuffer->depth--;

    if (head != NULL) {
        head->prev = currentItem;
    }

    item->type = JsonObject;
    item->child = head;

    inputBuffer->offset++;
    return true;

fail:
    if (head != NULL) {
        deleteJson(head);
    }

    return false;
}

/* Render an object to text. */
static JsonBool printObject(const Json * const item, PrintBuffer * const outputBuffer) {
    unsigned char *outputPointer = NULL;
    size_t length = 0;
    Json *currentItem = item->child;

    if (outputBuffer == NULL) {
        return false;
    }

    if (outputBuffer->depth >= JSON_NESTING_LIMIT) {
        return false; /* nesting is too deep */
    }

    /* Compose the output: */
    length = (size_t) (outputBuffer->format ? 2 : 1); /* fmt: {\n */
    outputPointer = ensure(outputBuffer, length + 1);
    if (outputPointer == NULL) {
        return false;
    }

    *outputPointer++ = '{';
    outputBuffer->depth++;
    if (outputBuffer->format) {
        *outputPointer++ = '\n';
    }
    outputBuffer->offset += length;

    while (currentItem) {
        if (outputBuffer->format) {
            size_t i;
            outputPointer = ensure(outputBuffer, outputBuffer->depth);
            if (outputPointer == NULL) {
                return false;
            }
            for (i = 0; i < outputBuffer->depth; i++) {
                *outputPointer++ = '\t';
            }
            outputBuffer->offset += outputBuffer->depth;
        }

        /* print key */
        if (!printStringPtr((unsigned char*)currentItem->string, outputBuffer)) {
            return false;
        }
        updateOffset(outputBuffer);

        length = (size_t) (outputBuffer->format ? 2 : 1);
        outputPointer = ensure(outputBuffer, length);
        if (outputPointer == NULL) {
            return false;
        }
        *outputPointer++ = ':';
        if (outputBuffer->format) {
            *outputPointer++ = '\t';
        }
        outputBuffer->offset += length;

        /* print value */
        if (!printValue(currentItem, outputBuffer)) {
            return false;
        }
        updateOffset(outputBuffer);

        /* print comma if not last */
        length = ((size_t)(outputBuffer->format ? 1 : 0) + (size_t)(currentItem->next ? 1 : 0));
        outputPointer = ensure(outputBuffer, length + 1);
        if (outputPointer == NULL) {
            return false;
        }
        if (currentItem->next) {
            *outputPointer++ = ',';
        }

        if (outputBuffer->format) {
            *outputPointer++ = '\n';
        }
        *outputPointer = '\0';
        outputBuffer->offset += length;

        currentItem = currentItem->next;
    }

    outputPointer = ensure(outputBuffer, outputBuffer->format ? (outputBuffer->depth + 1) : 2);
    if (outputPointer == NULL) {
        return false;
    }
    if (outputBuffer->format)  {
        size_t i;
        for (i = 0; i < (outputBuffer->depth - 1); i++) {
            *outputPointer++ = '\t';
        }
    }
    *outputPointer++ = '}';
    *outputPointer = '\0';
    outputBuffer->depth--;

    return true;
}

/* Get Array size/item / object item. */
int getJsonArraySize(const Json *array) {
    Json *child = NULL;
    size_t size = 0;

    if (array == NULL) {
        return 0;
    }

    child = array->child;

    while(child != NULL) {
        size++;
        child = child->next;
    }
    return (int)size;
}

static Json* getArrayItem(const Json *array, size_t index) {
    Json *currentChild = NULL;

    if (array == NULL) {
        return NULL;
    }

    currentChild = array->child;
    while ((currentChild != NULL) && (index > 0)) {
        index--;
        currentChild = currentChild->next;
    }

    return currentChild;
}

Json* getJsonArrayItem(const Json *array, int index) {
    if (index < 0) {
        return NULL;
    }

    return getArrayItem(array, (size_t)index);
}

static Json *getObjectItem(const Json * object, const char *name, const JsonBool caseSensitive) {
    Json *currentElement = NULL;

    if ((object == NULL) || (name == NULL)) {
        return NULL;
    }

    currentElement = object->child;
    if (caseSensitive) {
        while ((currentElement != NULL) && (currentElement->string != NULL) && (strcmp(name, currentElement->string) != 0)) {
            currentElement = currentElement->next;
        }
    } else {
        while ((currentElement != NULL) && (caseInsensitiveStrcmp((const unsigned char*)name, (const unsigned char*)(currentElement->string)) != 0)) {
            currentElement = currentElement->next;
        }
    }

    if ((currentElement == NULL) || (currentElement->string == NULL)) {
        return NULL;
    }

    return currentElement;
}

Json* getJsonObjectItem(const Json *object, const char *string) {
    return getObjectItem(object, string, false);
}

Json* getJsonObjectItemCaseSensitive(const Json *object, const char *string) {
    return getObjectItem(object, string, true);
}

JsonBool hasJsonObjectItem(const Json *object, const char *string) {
    return getJsonObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffixObject(Json *prev, Json *item) {
    prev->next = item;
    item->prev = prev;
}

/* Utility for handling references. */
static Json *createReference(const Json *item) {
    Json *reference = NULL;
    if (item == NULL) {
        return NULL;
    }

    reference = newJsonItem();
    if (reference == NULL) {
        return NULL;
    }

    memcpy(reference, item, sizeof(Json));
    reference->string = NULL;
    reference->type |= IsJsonReference;
    reference->next = reference->prev = NULL;
    return reference;
}

static JsonBool addItemToArray(Json *array, Json *item) {
    Json *child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item)){
        return false;
    }

    child = array->child;
    /*
     * To find the last item in array quickly, we use prev in array
     */
    if (child == NULL) {
        /* list is empty, start new one */
        array->child = item;
        item->prev = item;
        item->next = NULL;
    } else {
        /* append to the end */
        if (child->prev) {
            suffixObject(child->prev, item);
            array->child->prev = item;
        }
    }

    return true;
}

/* Add item to array/object. */
JsonBool addJsonItemToArray(Json *array, Json *item) {
    return addItemToArray(array, item);
}

/* helper function to cast away const */
static void* castAwayConst(const void* string) {
    return (void*)string;
}

static JsonBool addItemToObject(Json *object, const char *string, Json *item, const JsonBool constantKey) {
    char *newKey = NULL;
    int newType = JsonInvalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item)) {
        return false;
    }

    if (constantKey) {
        newKey = (char*)castAwayConst(string);
        newType = item->type | IsJsonStringConst;
    } else {
        newKey = (char*)Jsonstrdup((const unsigned char*)string);
        if (newKey == NULL) {
            return false;
        }

        newType = item->type & ~IsJsonStringConst;
    }

    if (!(item->type & IsJsonStringConst) && (item->string != NULL)) {
        free(item->string);
    }

    item->string = newKey;
    item->type = newType;

    return addItemToArray(object, item);
}

JsonBool addJsonItemToObject(Json *object, const char *string, Json *item) {
    return addItemToObject(object, string, item, false);
}

/* Add an item to an object with constant string as key */
JsonBool addJsonItemToObjectCS(Json *object, const char *string, Json *item) {
    return addItemToObject(object, string, item, true);
}

JsonBool addJsonItemReferenceToArray(Json *array, Json *item) {
    if (array == NULL) {
        return false;
    }
    return addItemToArray(array, createReference(item));
}

JsonBool addJsonItemReferenceToObject(Json *object, const char *string, Json *item) {
    if ((object == NULL) || (string == NULL)) {
        return false;
    }
    return addItemToObject(object, string, createReference(item), false);
}

Json* addJsonNullToObject(Json * const object, const char * const name) {
    Json *null = createJsonNull();
    if (addItemToObject(object, name, null, false)) {
        return null;
    }
    deleteJson(null);
    return NULL;
}

Json* addJsonTrueToObject(Json * const object, const char * const name) {
    Json *trueItem = createJsonTrue();
    if (addItemToObject(object, name, trueItem, false)) {
        return trueItem;
    }
    deleteJson(trueItem);
    return NULL;
}

Json* addJsonFalseToObject(Json * const object, const char * const name) {
    Json *falseItem = createJsonFalse();
    if (addItemToObject(object, name, falseItem, false)) {
        return falseItem;
    }
    deleteJson(falseItem);
    return NULL;
}

Json* addJsonBoolToObject(Json * const object, const char * const name, const JsonBool boolean) {
    Json *boolItem = createJsonBool(boolean);
    if (addItemToObject(object, name, boolItem, false)) {
        return boolItem;
    }

    deleteJson(boolItem);
    return NULL;
}

Json* addJsonNumberToObject(Json * const object, const char * const name, const double number) {
    Json *numberItem = createJsonNumber(number);
    if (addItemToObject(object, name, numberItem, false)) {
        return numberItem;
    }

    deleteJson(numberItem);
    return NULL;
}

Json* addJsonStringToObject(Json * const object, const char * const name, const char * const string) {
    Json *stringItem = createJsonString(string);
    if (addItemToObject(object, name, stringItem, false)) {
        return stringItem;
    }

    deleteJson(stringItem);
    return NULL;
}

Json* addJsonRawToObject(Json * const object, const char * const name, const char * const raw) {
    Json *rawItem = createJsonRaw(raw);
    if (addItemToObject(object, name, rawItem, false)) {
        return rawItem;
    }

    deleteJson(rawItem);
    return NULL;
}

Json* addJsonObjectToObject(Json * const object, const char * const name) {
    Json *objectItem = createJsonObject();
    if (addItemToObject(object, name, objectItem, false)) {
        return objectItem;
    }

    deleteJson(objectItem);
    return NULL;
}

Json* addJsonArrayToObject(Json * const object, const char * const name) {
    Json *array = createJsonArray();
    if (addItemToObject(object, name, array, false)) {
        return array;
    }

    deleteJson(array);
    return NULL;
}

Json* detachJsonItemViaPointer(Json *parent, Json * const item) {
    if ((parent == NULL) || (item == NULL) || (item != parent->child && item->prev == NULL)) {
        return NULL;
    }

    if (item != parent->child) {
        /* not the first element */
        item->prev->next = item->next;
    }
    if (item->next != NULL) {
        /* not the last element */
        item->next->prev = item->prev;
    }

    if (item == parent->child) {
        /* first element */
        parent->child = item->next;
    } else if (item->next == NULL) {
        /* last element */
        parent->child->prev = item->prev;
    }

    /* make sure the detached item doesn't point anywhere anymore */
    item->prev = NULL;
    item->next = NULL;

    return item;
}

Json* detachJsonItemFromArray(Json *array, int which) {
    if (which < 0)
    {
        return NULL;
    }

    return detachJsonItemViaPointer(array, getArrayItem(array, (size_t)which));
}

void deleteJsonItemFromArray(Json *array, int which) {
    deleteJson(detachJsonItemFromArray(array, which));
}

Json* detachJsonItemFromObject(Json *object, const char *string) {
    Json *toDetach = getJsonObjectItem(object, string);
    return detachJsonItemViaPointer(object, toDetach);
}

Json* detachJsonItemFromObjectCaseSensitive(Json *object, const char *string) {
    Json *toDetach = getJsonObjectItemCaseSensitive(object, string);
    return detachJsonItemViaPointer(object, toDetach);
}

void deleteJsonItemFromObject(Json *object, const char *string) {
    deleteJson(detachJsonItemFromObject(object, string));
}

void deleteJsonItemFromObjectCaseSensitive(Json *object, const char *string) {
    deleteJson(detachJsonItemFromObjectCaseSensitive(object, string));
}

/* Replace array/object items with new ones. */
JsonBool JsonInsertItemInArray(Json *array, int which, Json *newitem) {
    Json *afterInserted = NULL;

    if (which < 0 || newitem == NULL) {
        return false;
    }

    afterInserted = getArrayItem(array, (size_t)which);
    if (afterInserted == NULL) {
        return addItemToArray(array, newitem);
    }

    if (afterInserted != array->child && afterInserted->prev == NULL) {
        /* return false if afterInserted is a corrupted array item */
        return false;
    }

    newitem->next = afterInserted;
    newitem->prev = afterInserted->prev;
    afterInserted->prev = newitem;
    if (afterInserted == array->child) {
        array->child = newitem;
    } else {
        newitem->prev->next = newitem;
    }
    return true;
}

JsonBool JsonReplaceItemViaPointer(Json * const parent, Json * const item, Json * replacement) {
    if ((parent == NULL) || (parent->child == NULL) || (replacement == NULL) || (item == NULL)) {
        return false;
    }

    if (replacement == item) {
        return true;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL) {
        replacement->next->prev = replacement;
    }
    if (parent->child == item) {
        if (parent->child->prev == parent->child)
        {
            replacement->prev = replacement;
        }
        parent->child = replacement;
    } else {   /*
         * To find the last item in array quickly, we use prev in array.
         * We can't modify the last item's next pointer where this item was the parent's child
         */
        if (replacement->prev != NULL) {
            replacement->prev->next = replacement;
        }
        if (replacement->next == NULL) {
            parent->child->prev = replacement;
        }
    }

    item->next = NULL;
    item->prev = NULL;
    deleteJson(item);

    return true;
}

JsonBool JsonReplaceItemInArray(Json *array, int which, Json *newitem) {
    if (which < 0) {
        return false;
    }
    return JsonReplaceItemViaPointer(array, getArrayItem(array, (size_t)which), newitem);
}

static JsonBool replaceItemInObject(Json *object, const char *string, Json *replacement, JsonBool caseSensitive) {
    if ((replacement == NULL) || (string == NULL)) {
        return false;
    }

    /* replace the name in the replacement */
    if (!(replacement->type & IsJsonStringConst) && (replacement->string != NULL)) {
        free(replacement->string);
        replacement->string = NULL;
    }
    replacement->string = (char*)Jsonstrdup((const unsigned char*)string);
    if (replacement->string == NULL) {
        return false;
    }

    replacement->type &= ~IsJsonStringConst;

    return JsonReplaceItemViaPointer(object, getObjectItem(object, string, caseSensitive), replacement);
}

JsonBool JsonReplaceItemInObject(Json *object, const char *string, Json *newitem) {
    return replaceItemInObject(object, string, newitem, false);
}

JsonBool JsonReplaceItemInObjectCaseSensitive(Json *object, const char *string, Json *newitem) {
    return replaceItemInObject(object, string, newitem, true);
}

/* Create basic types: */
Json* createJsonNull(void) {
    Json *item = newJsonItem();
    if(item) {
        item->type = JsonNULL;
    }
    return item;
}

Json* createJsonTrue(void) {
    Json *item = newJsonItem();
    if(item) {
        item->type = JsonTrue;
    }
    return item;
}

Json* createJsonFalse(void) {
    Json *item = newJsonItem();
    if(item) {
        item->type = JsonFalse;
    }
    return item;
}

Json* createJsonBool(JsonBool boolean) {
    Json *item = newJsonItem();
    if(item) {
        item->type = boolean ? JsonTrue : JsonFalse;
    }
    return item;
}

Json* createJsonNumber(double num) {
    Json *item = newJsonItem();
    if(item) {
        item->type = JsonNumber;
        item->valueDouble = num;

        /* use saturation in case of overflow */
        if (num >= INT_MAX) {
            item->valueInt = INT_MAX;
        } else if (num <= (double)INT_MIN) {
            item->valueInt = INT_MIN;
        } else {
            item->valueInt = (int)num;
        }
    }
    return item;
}

Json* createJsonString(const char *string) {
    Json *item = newJsonItem();
    if(item) {
        item->type = JsonString;
        item->valueString = (char*)Jsonstrdup((const unsigned char*)string);
        if(!item->valueString) {
            deleteJson(item);
            return NULL;
        }
    }
    return item;
}

Json* createJsonStringReference(const char *string) {
    Json *item = newJsonItem();
    if (item != NULL) {
        item->type = JsonString | IsJsonReference;
        item->valueString = (char*)castAwayConst(string);
    }
    return item;
}

Json* createJsonObjectReference(const Json *child) {
    Json *item = newJsonItem();
    if (item != NULL) {
        item->type = JsonObject | IsJsonReference;
        item->child = (Json*)castAwayConst(child);
    }
    return item;
}

Json* createJsonArrayReference(const Json *child) {
    Json *item = newJsonItem();
    if (item != NULL) {
        item->type = JsonArray | IsJsonReference;
        item->child = (Json*)castAwayConst(child);
    }
    return item;
}

Json* createJsonRaw(const char *raw) {
    Json *item = newJsonItem();
    if(item) {
        item->type = JsonRaw;
        item->valueString = (char*)Jsonstrdup((const unsigned char*)raw);
        if(!item->valueString) {
            deleteJson(item);
            return NULL;
        }
    }
    return item;
}

Json*  createJsonArray(void) {
    Json *item = newJsonItem();
    if(item) {
        item->type=JsonArray;
    }
    return item;
}

Json* createJsonObject(void) {
    Json *item = newJsonItem();
    if (item) {
        item->type = JsonObject;
    }
    return item;
}

/* Create Arrays: */
Json* createJsonIntArray(const int *numbers, int count) {
    size_t i = 0;
    Json *n = NULL;
    Json *p = NULL;
    Json *a = NULL;

    if ((count < 0) || (numbers == NULL)) {
        return NULL;
    }

    a = createJsonArray();

    for(i = 0; a && (i < (size_t)count); i++) {
        n = createJsonNumber(numbers[i]);
        if (!n) {
            deleteJson(a);
            return NULL;
        }
        if(!i) {
            a->child = n;
        } else {
            suffixObject(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }
    return a;
}

Json* createJsonFloatArray(const float *numbers, int count) {
    size_t i = 0;
    Json *n = NULL;
    Json *p = NULL;
    Json *a = NULL;

    if ((count < 0) || (numbers == NULL)) {
        return NULL;
    }

    a = createJsonArray();

    for(i = 0; a && (i < (size_t)count); i++) {
        n = createJsonNumber((double)numbers[i]);
        if(!n) {
            deleteJson(a);
            return NULL;
        }
        if(!i) {
            a->child = n;
        } else {
            suffixObject(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }
    return a;
}

Json* createJsonDoubleArray(const double *numbers, int count) {
    size_t i = 0;
    Json *n = NULL;
    Json *p = NULL;
    Json *a = NULL;

    if ((count < 0) || (numbers == NULL)) {
        return NULL;
    }

    a = createJsonArray();

    for(i = 0; a && (i < (size_t)count); i++) {
        n = createJsonNumber(numbers[i]);
        if(!n) {
            deleteJson(a);
            return NULL;
        }
        if(!i) {
            a->child = n;
        } else {
            suffixObject(p, n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }
    return a;
}

Json* createJsonStringArray(const char *const *strings, int count) {
    size_t i = 0;
    Json *n = NULL;
    Json *p = NULL;
    Json *a = NULL;

    if ((count < 0) || (strings == NULL)) {
        return NULL;
    }

    a = createJsonArray();

    for (i = 0; a && (i < (size_t)count); i++) {
        n = createJsonString(strings[i]);
        if(!n) {
            deleteJson(a);
            return NULL;
        }
        if(!i) {
            a->child = n;
        } else {
            suffixObject(p,n);
        }
        p = n;
    }

    if (a && a->child) {
        a->child->prev = n;
    }
    return a;
}

/* Duplication */
Json* duplicateJsonRecurse(const Json *item, size_t depth, JsonBool recurse);

Json* duplicateJson(const Json *item, JsonBool recurse) {
    return duplicateJsonRecurse(item, 0, recurse );
}

Json * duplicateJsonRecurse(const Json *item, size_t depth, JsonBool recurse) {
    Json *newitem = NULL;
    Json *child = NULL;
    Json *next = NULL;
    Json *newchild = NULL;

    /* Bail on bad ptr */
    if (!item) {
        goto fail;
    }
    /* Create new item */
    newitem = newJsonItem();
    if (!newitem) {
        goto fail;
    }
    /* Copy over all vars */
    newitem->type = item->type & (~IsJsonReference);
    newitem->valueInt = item->valueInt;
    newitem->valueDouble = item->valueDouble;
    if (item->valueString) {
        newitem->valueString = (char*)Jsonstrdup((unsigned char*)item->valueString);
        if (!newitem->valueString)
        {
            goto fail;
        }
    }
    if (item->string) {
        newitem->string = (item->type&IsJsonStringConst) ? item->string : (char*)Jsonstrdup((unsigned char*)item->string);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    /* If non-recursive, then we're done! */
    if (!recurse) {
        return newitem;
    }
    /* Walk the ->next chain for the child. */
    child = item->child;
    while (child != NULL) {
        if(depth >= JSON_CIRCULAR_LIMIT) {
            goto fail;
        }
        newchild = duplicateJsonRecurse(child, depth + 1, true); /* Duplicate (with recurse) each item in the ->next chain */
        if (!newchild) {
            goto fail;
        }
        if (next != NULL) {
            /* If newitem->child already set, then crosswire ->prev and ->next and move on */
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        } else {
            /* Set newitem->child and move to it */
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }
    if (newitem && newitem->child) {
        newitem->child->prev = newchild;
    }

    return newitem;

fail:
    if (newitem != NULL) {
        deleteJson(newitem);
    }
    return NULL;
}

static void skipOnelineComment(char **input) {
    *input += staticStrlen("//");
    for (; (*input)[0] != '\0'; ++(*input)) {
        if ((*input)[0] == '\n') {
            *input += staticStrlen("\n");
            return;
        }
    }
}

static void skipMultilineComment(char **input) {
    *input += staticStrlen("/*");

    for (; (*input)[0] != '\0'; ++(*input)) {
        if (((*input)[0] == '*') && ((*input)[1] == '/')) {
            *input += staticStrlen("*/");
            return;
        }
    }
}

static void minifyString(char **input, char **output) {
    (*output)[0] = (*input)[0];
    *input += staticStrlen("\"");
    *output += staticStrlen("\"");


    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output)) {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"') {
            (*output)[0] = '\"';
            *input += staticStrlen("\"");
            *output += staticStrlen("\"");
            return;
        } else if (((*input)[0] == '\\') && ((*input)[1] == '\"')) {
            (*output)[1] = (*input)[1];
            *input += staticStrlen("\"");
            *output += staticStrlen("\"");
        }
    }
}

void minifyJson(char *json) {
    char *into = json;
    if (json == NULL) {
        return;
    }

    while (json[0] != '\0') {
        switch (json[0]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                json++;
                break;

            case '/':
                if (json[1] == '/')
                {
                    skipOnelineComment(&json);
                }
                else if (json[1] == '*')
                {
                    skipMultilineComment(&json);
                } else {
                    json++;
                }
                break;

            case '\"':
                minifyString(&json, (char**)&into);
                break;

            default:
                into[0] = json[0];
                json++;
                into++;
        }
    }

    /* and null-terminate. */
    *into = '\0';
}

JsonBool isJsonInvalid(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonInvalid;
}

JsonBool isJsonFalse(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonFalse;
}

JsonBool isJsonTrue(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xff) == JsonTrue;
}

JsonBool isJsonBool(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & (JsonTrue | JsonFalse)) != 0;
}

JsonBool isJsonNull(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonNULL;
}

JsonBool isJsonNumber(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonNumber;
}

JsonBool isJsonString(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonString;
}

JsonBool isJsonArray(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonArray;
}

JsonBool isJsonObject(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonObject;
}

JsonBool isJsonRaw(const Json *item) {
    if (item == NULL) {
        return false;
    }
    return (item->type & 0xFF) == JsonRaw;
}

JsonBool compareJson(const Json * const a, const Json * const b, const JsonBool caseSensitive) {
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF))) {
        return false;
    }

    /* check if type is valid */
    switch (a->type & 0xFF) {
        case JsonFalse:
        case JsonTrue:
        case JsonNULL:
        case JsonNumber:
        case JsonString:
        case JsonRaw:
        case JsonArray:
        case JsonObject:
            break;

        default:
            return false;
    }

    /* identical objects are equal */
    if (a == b) {
        return true;
    }

    switch (a->type & 0xFF) {
        /* in these cases and equal type is enough */
        case JsonFalse:
        case JsonTrue:
        case JsonNULL:
            return true;

        case JsonNumber:
            if (compareDouble(a->valueDouble, b->valueDouble)) {
                return true;
            }
            return false;

        case JsonString:
        case JsonRaw:
            if ((a->valueString == NULL) || (b->valueString == NULL)) {
                return false;
            }
            if (strcmp(a->valueString, b->valueString) == 0) {
                return true;
            }

            return false;

        case JsonArray: {
            Json *aElement = a->child;
            Json *bElement = b->child;

            for (; (aElement != NULL) && (bElement != NULL);) {
                if (!compareJson(aElement, bElement, caseSensitive)) {
                    return false;
                }

                aElement = aElement->next;
                bElement = bElement->next;
            }

            /* one of the arrays is longer than the other */
            if (aElement != bElement) {
                return false;
            }

            return true;
        }

        case JsonObject: {
            Json *aElement = NULL;
            Json *bElement = NULL;
            JsonArrayForEach(aElement, a) {
                /* TODO This has O(n^2) runtime, which is horrible! */
                bElement = getObjectItem(b, aElement->string, caseSensitive);
                if (bElement == NULL) {
                    return false;
                }

                if (!compareJson(aElement, bElement, caseSensitive)) {
                    return false;
                }
            }

            /* doing this twice, once on a and b to prevent true comparison if a subset of b
             * TODO: Do this the proper way, this is just a fix for now */
            JsonArrayForEach(bElement, b) {
                aElement = getObjectItem(a, bElement->string, caseSensitive);
                if (aElement == NULL) {
                    return false;
                }

                if (!compareJson(bElement, aElement, caseSensitive)) {
                    return false;
                }
            }
            return true;
        }

        default:
            return false;
    }
}