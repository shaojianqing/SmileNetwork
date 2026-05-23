#define TYPE_NONE               0
#define TYPE_FOLAT              1
#define TYPE_VECTOR             2
#define TYPE_METRIX             3

#define SUCCESS                 0
#define MEMORY_ALLOCATE_ERROR   1
#define INSTANCE_IS_NULL        2
#define MATRIX_NOT_MATCH        3
#define VECTOR_NOT_MATCH        4

typedef struct Result Result;

struct Result {

    int code;

    int type;

    String *message;

    Object *data;

    float value;

    bool (*success)(Result *this);

    Object* (*getData)(Result *this);

    float (*getValue)(Result *this);
};

Result* createResultWithData(int code, String *message, int type, Object *data);

Result* createResultWithValue(int code, String *message, float value);

Result* createResultWithoutData(int code, String *message);

void releaseResult(Result* this);