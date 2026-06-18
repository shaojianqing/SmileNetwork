

typedef void (*Optimizer)(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);


void SGDOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);

void MomentumOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);

void AdaGradOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);

void AdamOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);