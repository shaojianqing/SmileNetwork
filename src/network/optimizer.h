

typedef Result* (*Optimizer)(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);


Result* SGDOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);

Result* MomentumOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);

Result* AdaGradOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);

Result* AdamOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate);