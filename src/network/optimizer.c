#include <stdlib.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../result/result.h"
#include "../random/random.h"

#include "bias.h"
#include "matrix.h"
#include "optimizer.h"

Result* SGDOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                     Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {

    Result *result = mulMatrixNumber(gradientMatrix, learnRate);
    if (!success(result)) {
        return result;
    }
    releaseResult(result);

    
    result = subMatrix(modelMatrix, gradientMatrix);
    if (!success(result)) {
        return result;
    }
    releaseResult(result);

    ;
    result = mulBiasNumber(gradientBias, learnRate);
    if (!success(result)) {
        return result;
    }
    releaseResult(result);

    
    result = subBias(modelBias, gradientBias);
    if (!success(result)) {
        return result;
    }
    releaseResult(result);

    return createResultWithoutData(SUCCESS, NULL);
}

Result* MomentumOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                          Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {
    return createResultWithoutData(SUCCESS, NULL);
}

Result* AdaGradOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                         Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {
    return createResultWithoutData(SUCCESS, NULL);
}

Result* AdamOptimizer(Matrix *modelMatrix, Bias *modelBias, 
                             Matrix *gradientMatrix, Bias *gradientBias, float learnRate) {
    return createResultWithoutData(SUCCESS, NULL); 
}