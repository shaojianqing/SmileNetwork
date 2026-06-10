#include <stdio.h>

#include "common/common.h"
#include "common/constant.h"
#include "memory/memory.h"
#include "json/json.h"
#include "datatype/datatype.h"
#include "datatype/stringtype.h"
#include "logger/logger.h"
#include "random/random.h"
#include "command/command.h"
#include "result/result.h"
#include "network/bias.h"
#include "network/vector.h"
#include "network/activator.h"

int main(int argc, char **argv) {

    initMemoryConfig();
    initRandomSeed();
    initActivatorMap();
    initLoggerConfig();
    initCommandConfig();
    
    showNetworkInfo();
    runCommandEvent();
}
