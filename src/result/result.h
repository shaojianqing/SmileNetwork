#define TYPE_NONE               0
#define TYPE_FLOAT              1
#define TYPE_BYTE_BUFFER        2
#define TYPE_CHAR_BUFFER        3
#define TYPE_VECTOR             4
#define TYPE_METRIX             5
#define TYPE_NETWORK_CONFIG     6

#define SUCCESS                 0
#define MEMORY_ALLOCATE_ERROR   1
#define INSTANCE_IS_NULL        2
#define MATRIX_NOT_MATCH        3
#define VECTOR_NOT_MATCH        4
#define LOSSFUNC_NO_CONFIG      5
#define GRADFUNC_NO_CONFIG      6
#define FILE_OPEN_ERROR         7
#define FILE_READ_ERROR         8
#define CONFIG_NO_EXIST         9

typedef struct Result Result;

struct Result {

    int code;

    int type;

    char *message;

    Object *data;

    float value;

    bool (*success)(Result *this);

    Object* (*getData)(Result *this);

    float (*getValue)(Result *this);
};

Result* createResultWithData(int code, char *message, int type, Object *data);

Result* createResultWithValue(int code, char *message, float value);

Result* createResultWithoutData(int code, char *message);

void releaseResult(Result* this);