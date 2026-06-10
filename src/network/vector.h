
typedef struct Vector Vector;

typedef struct Matrix Matrix;

Vector* createVector(int count);

void releaseVector(Vector *this);

int getElementCount(Vector *this);

Result* mulScalar(Vector *this, Vector *vector);

Result* mulTensor(Vector *this, Vector *target);

Result* mulHadamard(Vector *this, Vector *vector);

Result* mulMatrixVector(Vector *this, Matrix *matrix);

Result* addVector(Vector *this, Vector *vector);

Result* addBias(Vector *this, Bias *bias);

Result* copyVector(Vector *this, Vector *target);

float getVectorValue(Vector *this, int index);

void setVectorValue(Vector *this, int index, float value);
