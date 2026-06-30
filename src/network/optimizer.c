#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../generator/generator.h"

#include "bias.h"
#include "matrix.h"
#include "optimizer.h"

void SGDOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                     Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {

    mulMatrixNumber(gradientMatrix, learnRate);
    subMatrix(modelMatrix, gradientMatrix);

    mulBiasNumber(gradientBias, learnRate);
    subBias(modelBias, gradientBias);
}

void MomentumOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                          Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {
}

void AdaGradOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                         Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {
}

void AdamOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {
}