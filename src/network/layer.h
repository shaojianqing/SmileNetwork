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

Result* forward(BaseLayer *this, Vector *vector);

Result* backward(BaseLayer *this, Vector *target);

Result* optimize(BaseLayer *this, float learnRate);

Result* input(InputLayer *this, Vector *vector);

Result* loss(OutputLayer *this, Vector *expect);

Result* output(OutputLayer *this);

void setNextLayer(BaseLayer *this, BaseLayer *next);

void setPrevLayer(BaseLayer *this, BaseLayer *prev);
