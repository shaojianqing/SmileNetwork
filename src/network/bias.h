typedef struct Bias Bias;

typedef struct Vector Vector;

Bias* createBias(int dimensionCount, Generator generator);

void releaseBias(Bias *this);

int getBiasElementCount(Bias *this);

void copyBias(Bias *this, Vector *vector);

void subBias(Bias *this, Bias *bias);

void mulBiasNumber(Bias *this, float number);

float getBiasValue(Bias *this, int index);

void setBiasValue(Bias *this, int index, float value);