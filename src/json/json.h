#include <stddef.h>

/* Json Types: */
#define JsonInvalid (0)
#define JsonFalse  (1 << 0)
#define JsonTrue   (1 << 1)
#define JsonNULL   (1 << 2)
#define JsonNumber (1 << 3)
#define JsonString (1 << 4)
#define JsonArray  (1 << 5)
#define JsonObject (1 << 6)
#define JsonRaw    (1 << 7) /* raw json */

#define IsJsonReference 256
#define IsJsonStringConst 512

/* Limits how deeply nested arrays/objects can be before Json rejects to parse them.
 * This is to prevent stack overflows. */
#define JSON_NESTING_LIMIT 1000

/* Limits the length of circular references can be before Json rejects to parse them.
 * This is to prevent stack overflows. */
#define JSON_CIRCULAR_LIMIT 10000

typedef int JsonBool;

typedef struct Json Json;

/* Memory Management: the caller is always responsible to free the results from all variants of JsonParse (with JsonDelete) and JsonPrint (with stdlib free). The exception is JsonPrintPreallocated, where the caller has full responsibility of the buffer. */
/* Supply a block of JSON, and this returns a Json object you can interrogate. */
Json* parseJson(const char *value);
Json* parseJsonWithLength(const char *value, size_t buffer_length);
/* ParseWithOpts allows you to require (and check) that the JSON is null terminated, and to retrieve the pointer to the final byte parsed. */
/* If you supply a ptr in return_parse_end and parsing fails, then return_parse_end will contain a pointer to the error so will match JsonGetErrorPtr(). */
Json* parseJsonWithOpts(const char *value, const char **returnParseEnd, JsonBool requireNullTerminated);
Json* parseJsonWithLengthOpts(const char *value, size_t bufferLength, const char **returnParseEnd, JsonBool requireNullTerminated);

/* Render a Json entity to text for transfer/storage. */
char* printJson(const Json *item);
/* Render a Json entity to text for transfer/storage without any formatting. */
char* printJsonUnformatted(const Json *item);
/* Render a Json entity to text using a buffered strategy. prebuffer is a guess at the final size. guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted */
char* printJsonBuffered(const Json *item, int prebuffer, JsonBool fmt);
/* Render a Json entity to text using a buffer already allocated in memory with given length. Returns 1 on success and 0 on failure. */
/* NOTE: Json is not always 100% accurate in estimating how much memory it will use, so to be safe allocate 5 bytes more than you actually need */
JsonBool printJsonPreallocated(Json *item, char *buffer, const int length, const JsonBool format);
/* Delete a Json entity and all subentities. */
void deleteJson(Json *item);

/* Returns the number of items in an array (or object). */
int getJsonArraySize(const Json *array);
/* Retrieve item number "index" from array "array". Returns NULL if unsuccessful. */
Json* getJsonArrayItem(const Json *array, int index);
/* Get item "string" from object. Case insensitive. */
Json* getJsonObjectItem(const Json *object, const char *string);
Json* getJsonObjectItemCaseSensitive(const Json *object, const char *string);
JsonBool hasJsonObjectItem(const Json *object, const char *string);
/* For analysing failed parses. This returns a pointer to the parse error. You'll probably need to look a few chars back to make sense of it. Defined when JsonParse() returns 0. 0 when JsonParse() succeeds. */
const char* getJsonErrorPtr(void);

/* Check item type and return its value */
char* getJsonStringValue(const Json *item);
double getJsonNumberValue(const Json *item);

/* These functions check the type of an item */
JsonBool isJsonInvalid(const Json *item);
JsonBool isJsonFalse(const Json *item);
JsonBool isJsonTrue(const Json *item);
JsonBool isJsonBool(const Json *item);
JsonBool isJsonNull(const Json *item);
JsonBool isJsonNumber(const Json *item);
JsonBool isJsonString(const Json *item);
JsonBool isJsonArray(const Json *item);
JsonBool isJsonObject(const Json *item);
JsonBool isJsonRaw(const Json *item);

/* These calls create a Json item of the appropriate type. */
Json* createJsonNull(void);
Json* createJsonTrue(void);
Json* createJsonFalse(void);
Json* createJsonBool(JsonBool boolean);
Json* createJsonNumber(double num);
Json* createJsonString(const char *string);
/* raw json */
Json* createJsonRaw(const char *raw);
Json* createJsonArray(void);
Json* createJsonObject(void);

/* Create a string where valuestring references a string so
 * it will not be freed by JsonDelete */
Json* createJsonStringReference(const char *string);
/* Create an object/array that only references it's elements so
 * they will not be freed by JsonDelete */
Json* createJsonObjectReference(const Json *child);
Json* createJsonArrayReference(const Json *child);

/* These utilities create an Array of count items.
 * The parameter count cannot be greater than the number of elements in the number array, otherwise array access will be out of bounds.*/
Json* createJsonIntArray(const int *numbers, int count);
Json* createJsonFloatArray(const float *numbers, int count);
Json* createJsonDoubleArray(const double *numbers, int count);
Json* createJsonStringArray(const char *const *strings, int count);

/* Append item to the specified array/object. */
JsonBool addJsonItemToArray(Json *array, Json *item);
JsonBool addJsonItemToObject(Json *object, const char *string, Json *item);
/* Use this when string is definitely const (i.e. a literal, or as good as), and will definitely survive the Json object.
 * WARNING: When this function was used, make sure to always check that (item->type & IsJsonStringConst) is zero before
 * writing to `item->string` */
JsonBool addJsonItemToObjectCS(Json *object, const char *string, Json *item);
/* Append reference to item to the specified array/object. Use this when you want to add an existing Json to a new Json, but don't want to corrupt your existing Json. */
JsonBool addJsonItemReferenceToArray(Json *array, Json *item);
JsonBool addJsonItemReferenceToObject(Json *object, const char *string, Json *item);

/* Detach items from Arrays/Objects. */
Json* detachJsonItemViaPointer(Json *parent, Json *item);
Json* detachJsonItemFromArray(Json *array, int which);
Json* detachJsonItemFromObject(Json *object, const char *string);
Json* detachJsonItemFromObjectCaseSensitive(Json *object, const char *string);

/* Remove items from Arrays/Objects. */
void deleteJsonItemFromArray(Json *array, int which);
void deleteJsonItemFromObject(Json *object, const char *string);
void deleteJsonItemFromObjectCaseSensitive(Json *object, const char *string);

/* Update array items. */
JsonBool JsonInsertItemInArray(Json *array, int which, Json *newitem); /* Shifts pre-existing items to the right. */
JsonBool JsonReplaceItemViaPointer(Json *parent, Json *item, Json * replacement);
JsonBool JsonReplaceItemInArray(Json *array, int which, Json *newitem);
JsonBool JsonReplaceItemInObject(Json *object,const char *string,Json *newitem);
JsonBool JsonReplaceItemInObjectCaseSensitive(Json *object,const char *string,Json *newitem);

/* Duplicate a Json item */
Json* duplicateJson(const Json *item, JsonBool recurse);
/* Duplicate will create a new, identical Json item to the one you pass, in new memory that will
 * need to be released. With recurse!=0, it will duplicate any children connected to the item.
 * The item->next and ->prev pointers are always zero on return from Duplicate. */
/* Recursively compare two Json items for equality. If either a or b is NULL or invalid, they will be considered unequal.
 * case_sensitive determines if object keys are treated case sensitive (1) or case insensitive (0) */
JsonBool compareJson(const Json *a, const Json *b, const JsonBool case_sensitive);

/* Minify a strings, remove blank characters(such as ' ', '\t', '\r', '\n') from strings.
 * The input pointer json cannot point to a read-only address area, such as a string constant, 
 * but should point to a readable and writable address area. */
void minifyJson(char *json);

/* Helper functions for creating and adding items to an object at the same time.
 * They return the added item or NULL on failure. */
Json* addJsonNullToObject(Json *object, const char *name);
Json* addJsonTrueToObject(Json *object, const char *name);
Json* addJsonFalseToObject(Json *object, const char *name);
Json* addJsonBoolToObject(Json *object, const char *name, const JsonBool boolean);
Json* addJsonNumberToObject(Json *object, const char *name, const double number);
Json* addJsonStringToObject(Json *object, const char *name, const char *string);
Json* addJsonRawToObject(Json *object, const char *name, const char *raw);
Json* addJsonObjectToObject(Json *object, const char *name);
Json* addJsonArrayToObject(Json *object, const char *name);

/* When assigning an integer value, it needs to be propagated to valuedouble too. */
#define setJsonIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
/* helper for the JsonSetNumberValue macro */
double setJsonNumberHelper(Json *object, double number);
#define setJsonNumberValue(object, number) ((object != NULL) ? JsonSetNumberHelper(object, (double)number) : (number))
/* Change the valuestring of a JsonString object, only takes effect when type of object is JsonString */
char* setJsonValueString(Json *object, const char *valuestring);

/* If the object is not a boolean type this does nothing and returns JsonInvalid else it returns the new type*/
#define setJsonBoolValue(object, boolValue) ( \
    (object != NULL && ((object)->type & (JsonFalse|JsonTrue))) ? \
    (object)->type=((object)->type &(~(JsonFalse|JsonTrue)))|((boolValue)?JsonTrue:JsonFalse) : \
    JsonInvalid\
)

/* Macro for iterating over an array or object */
#define JsonArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)
