#include <stdlib.h>

#include "../common/common.h"
#include "../datatype/stringtype.h"

#include "bias.h"
#include "result.h"
#include "vector.h"
#include "loss.h"

static float mse(Vector *source, Vector *target);

static float cel(Vector *source, Vector *target);

Loss getLoss(LossKind kind) {
    if (kind == MSE) {
        return mse;
    } else if (kind == CEL) {
        return cel;
    }
    return NULL;
}

static float mse(Vector *source, Vector *target) {
    return 0.0;
}

static float cel(Vector *source, Vector *target) {
    return 0.0;
}