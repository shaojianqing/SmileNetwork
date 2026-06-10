typedef struct Matrix Matrix;

Matrix* createMatrix(int rowCount, int columnCount, Random random);

void releaseMatrix(Matrix *matrix);

int getRowCount(Matrix *this);

int getColumnCount(Matrix *this);

Result* mulVector(Matrix *this, Vector *vector);

Result* addMatrix(Matrix *this, Matrix *matrix);

Result* subMatrix(Matrix *this, Matrix *matrix);

Result* mulMatrix(Matrix *this, Matrix *matrix);

Result* mulMatrixNumber(Matrix *this, float number);

float getMatrixValue(Matrix *this, int row, int column);

void setMatrixValue(Matrix *this, int row, int column, float value);