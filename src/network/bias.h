typedef struct Bias Bias;

struct Bias {

    float *elements;

    int count;

    float (*getValue)(Bias *this, int index);

    void (*setValue)(Bias *this, int index, float value);
};

Bias *createBias(int dimensionCount);

void releaseBias(Bias *this);