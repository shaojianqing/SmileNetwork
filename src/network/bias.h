typedef struct Bias Bias;

typedef struct Vector Vector;

struct Bias {

    float *elements;

    int count;

    float (*getValue)(Bias *this, int index);

    void (*setValue)(Bias *this, int index, float value);

    Result* (*copy)(Bias *this, Vector *vector);

    Result* (*subBias)(Bias *this, Bias *bias);

    Result* (*mulNumber)(Bias *this, float number);
};

Bias *createBias(int dimensionCount, Random random);

void releaseBias(Bias *this);