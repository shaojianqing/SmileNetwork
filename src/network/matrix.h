typedef struct Matrix Matrix;

Matrix* createMatrix(int rowCount, int columnCount, Random random);

void releaseMatrix(Matrix *matrix);

int getRowCount(Matrix *this);

int getColumnCount(Matrix *this);

Vector* mulVector(Matrix *this, Vector *vector);

void addMatrix(Matrix *this, Matrix *matrix);

void subMatrix(Matrix *this, Matrix *matrix);

void mulMatrixNumber(Matrix *this, float number);

Matrix* mulMatrix(Matrix *this, Matrix *matrix);

float getMatrixValue(Matrix *this, int row, int column);

void setMatrixValue(Matrix *this, int row, int column, float value);