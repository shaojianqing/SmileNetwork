typedef struct Matrix Matrix;

struct Matrix {

    float* data;

    int rowCount;

    int columnCount;

    Result* (*mulVector)(Matrix *this, Vector *vector);

    Result* (*addMatrix)(Matrix *this, Matrix *metrix);

    Result* (*subMatrix)(Matrix *this, Matrix *metrix);

    Result* (*mulMatrix)(Matrix *this, Matrix *metrix);

    Result* (*mulNumber)(Matrix *this, float number);

    Result* (*transpose)(Matrix *this);

    void (*setValue)(Matrix *this, int row, int column, float value);
};

Matrix* createMatrix(int rowCount, int columnCount, Random random);

void releaseMatrix(Matrix *matrix);