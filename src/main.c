#include <stdio.h>

#include "common/common.h"
#include "common/constant.h"
#include "datatype/datatype.h"
#include "datatype/stringtype.h"
#include "command/command.h"


int main(int argc, char **argv) {

    showNetworkInfo();
    showCommandInfo();
    initCommandConfig();
    runCommandEvent();
}
