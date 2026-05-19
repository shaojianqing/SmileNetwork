typedef struct Matrix Matrix;

struct Matrix {

    float** matrix;

    int rowCount;

    int columnCount;

    int reference;

    Result* (*mulVector)(Matrix *this, Vector *vector);

    Result* (*addMatrix)(Matrix *this, Matrix *metrix);

    Result* (*mulMatrix)(Matrix *this, Matrix *metrix);

    void (*retain)(Matrix *this);

    void (*release)(Matrix *this);
};

Matrix* createMatrix();