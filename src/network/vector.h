
typedef struct Vector Vector;

struct Vector {

    float *elements;

    int count;

    Result* (*mul)(Vector *this, Vector *vector);

    Result* (*add)(Vector *this, Vector *vector);

    void (*retain)(Vector *this);

    void (*release)(Vector *this);
};

Vector *createVector(float *elements, int count);
