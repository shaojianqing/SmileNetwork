#include <memory.h>
#include <stdlib.h>

#include "../common/common.h"

#include "result.h"
#include "vector.h"
#include "matrix.h"
#include "activator.h"
#include "layer.h"

static Result* input(Layer *this, Vector *vector);

static Result* output(Layer *this, Vector *vector);

static void retain(Layer *this);

static void release(Layer *this);

Layer *createLayer(Matrix *matrix, Activator activator) {
    Layer *layer = (Layer*)malloc(sizeof(Layer));
    if (layer != NULL) {
        layer->matrix = matrix;
        layer->activator = activator;

        layer->input = input;
        layer->output = output;

        layer->retain = retain;
        layer->release = release;
    }
    return layer;
}

static Result* input(Layer *this, Vector *vector) {
    return NULL;
}

static Result* output(Layer *this, Vector *vector) {
    return NULL;
}

static void retain(Layer *this) {

}

static void release(Layer *this) {

}