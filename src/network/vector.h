
typedef struct Vector Vector;

typedef struct Matrix Matrix;

Vector* createVector(int count);

void releaseVector(Vector *this);

int getElementCount(Vector *this);

float mulScalar(Vector *this, Vector *vector);

Matrix* mulTensor(Vector *this, Vector *target);

Vector* mulHadamard(Vector *this, Vector *vector);

Vector* mulMatrixVector(Vector *this, Matrix *matrix);

void addVector(Vector *this, Vector *vector);

void addBias(Vector *this, Bias *bias);

void copyVector(Vector *this, Vector *target);

float getVectorValue(Vector *this, int index);

void setVectorValue(Vector *this, int index, float value);
