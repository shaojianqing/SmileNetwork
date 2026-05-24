#include <stdio.h>

#include "common/common.h"
#include "common/constant.h"
#include "datatype/datatype.h"
#include "datatype/stringtype.h"
#include "logger/logger.h"
#include "command/command.h"
#include "network/bias.h"
#include "network/result.h"
#include "network/vector.h"
#include "network/activator.h"

#include "json/json.h"

int main(int argc, char **argv) {

    initLoggerConfig();
    initActivatorMap();
    
    showNetworkInfo();
    showCommandInfo();
    initCommandConfig();
    runCommandEvent();
}
