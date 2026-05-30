#include <stdlib.h>

#include "../common/common.h"
#include "../memory/memory.h"

#include "label.h"

Label *createLabel() {
    Label *label = (Label*)allocate(sizeof(Label));
    if (label != NULL) {

    }
    return label;
}