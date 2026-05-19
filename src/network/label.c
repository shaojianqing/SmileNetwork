#include <memory.h>
#include <stdlib.h>

#include "label.h"

static void retain(Label *this);

static void release(Label *this);

Label *createLabel() {
    Label *label = (Label*)malloc(sizeof(Label));
    if (label != NULL) {

        label->retain = retain;
        label->release = release;
    }
    return label;
}

static void retain(Label *this) {

}

static void release(Label *this) {
    
}