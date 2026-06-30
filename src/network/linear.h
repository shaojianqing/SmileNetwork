typedef struct VectorLayer VectorLayer;

typedef struct InputLayer InputLayer;

typedef struct OutputLayer OutputLayer;

typedef struct AffineLayer AffineLayer;

InputLayer* buildInputLayer(LinearLayerConfig *config);

OutputLayer* buildOutputLayer(LinearLayerConfig *config);

AffineLayer* buildAffineLayer(LinearLayerConfig *config);

void releaseInputLayer(InputLayer *inputLayer);

void releaseOutputLayer(OutputLayer *outputLayer);

void releaseAffineLayer(AffineLayer *affineLayer);

void forward(VectorLayer *this, Vector *vector);

void backward(VectorLayer *this, Vector *target);

void optimize(VectorLayer *this, float learnRate);

void input(InputLayer *this, Vector *vector);

float loss(OutputLayer *this, Vector *expect);

Vector* output(OutputLayer *this);

void setNextLayer(VectorLayer *this, VectorLayer *next);

void setPrevLayer(VectorLayer *this, VectorLayer *prev);
