typedef struct Matrix Matrix;

struct Matrix {

    float* data;

    int rowCount;

    int columnCount;

    Result* (*mulVector)(Matrix *this, Vector *vector);

    Result* (*addMatrix)(Matrix *this, Matrix *metrix);

    Result* (*subMatrix)(Matrix *this, Matrix *metrix);

    Result* (*mulMatrix)(Matrix *this, Matrix *metrix);
};

Matrix* createMatrix(int rowCount, int columnCount);

void releaseMatrix(Matrix *matrix);