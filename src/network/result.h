#define TYPE_FOLAT              1
#define TYPE_VECTOR             2
#define TYPE_METRIX             3

typedef union ResulData ResulData;

typedef struct Result Result;

union ResulData {

    float floatData;

    void *objectData;
};

struct Result {

    int code;

    char *message;

    int type;

    ResulData data;

    bool (*success)(Result *this);

    void (*retain)(Result *this);

    void (*release)(Result *this);
};

Result* createResult(int code, char *message, int type, ResulData data);