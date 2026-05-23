
typedef struct Vector Vector;

struct Vector {

    float *elements;

    int count;

    Result* (*mul)(Vector *this, Vector *vector);

    Result* (*add)(Vector *this, Vector *vector);

    Result* (*addBias)(Vector *this, Bias *bias);

    float (*getValue)(Vector *this, int index);

    void (*setValue)(Vector *this, int index, float value);
};

Vector *createVector(int count);
