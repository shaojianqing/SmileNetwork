
typedef struct Vector Vector;

struct Vector {

    float *elements;

    int count;

    Result* (*mul)(Vector *this, Vector *vector);

    Result* (*mulHamd)(Vector *this, Vector *vector);

    Result* (*add)(Vector *this, Vector *vector);

    Result* (*addBias)(Vector *this, Bias *bias);

    Result* (*copy)(Vector *this, Vector *target);

    Result* (*matrixMul)(Vector *this, Vector *target);

    void (*printVector)(Vector *this, char *message, int limit);

    float (*getValue)(Vector *this, int index);

    void (*setValue)(Vector *this, int index, float value);
};

Vector *createVector(int count);

void releaseVector(Vector *this);
