typedef struct BaseLayer BaseLayer;

typedef struct InputLayer InputLayer;

typedef struct OutputLayer OutputLayer;

typedef struct LinearLayer LinearLayer;

InputLayer* buildInputLayer(LayerConfig *config);

OutputLayer* buildOutputLayer(LayerConfig *config);

LinearLayer* buildLinearLayer(LayerConfig *config);

void releaseInputLayer(InputLayer *inputLayer);

void releaseOutputLayer(OutputLayer *outputLayer);

void releaseLinearLayer(LinearLayer *linearLayer);

void forward(BaseLayer *this, Vector *vector);

void backward(BaseLayer *this, Vector *target);

void optimize(BaseLayer *this, float learnRate);

void input(InputLayer *this, Vector *vector);

float loss(OutputLayer *this, Vector *expect);

Vector* output(OutputLayer *this);

void setNextLayer(BaseLayer *this, BaseLayer *next);

void setPrevLayer(BaseLayer *this, BaseLayer *prev);
