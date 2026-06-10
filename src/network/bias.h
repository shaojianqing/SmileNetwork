typedef struct Bias Bias;

typedef struct Vector Vector;

Bias* createBias(int dimensionCount, Random random);

void releaseBias(Bias *this);

int getBiasElementCount(Bias *this);

Result* copyBias(Bias *this, Vector *vector);

Result* subBias(Bias *this, Bias *bias);

Result* mulBiasNumber(Bias *this, float number);

float getBiasValue(Bias *this, int index);

void setBiasValue(Bias *this, int index, float value);