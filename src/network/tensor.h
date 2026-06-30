typedef struct Tensor Tensor;

Tensor* createTensor(const int dimension, const int shape[], Generator generator);

void releaseTensor(Tensor *tensor);

int getDimension(Tensor *this);

int getElementSize(Tensor *this);

int* getTensorShape(Tensor *this);

float mulTensorScalar(Tensor *this, Tensor *target);

void mulTensorNumber(Tensor *this, float number);

void addTensor(Tensor *this, Tensor *target);

void subTensor(Tensor *this, Tensor *target);

Vector* flattenTenosr(Tensor *this);

float getTensorValue(Tensor *this, const int index[]);

void setTensorValue(Tensor *this, const int index[], float value);