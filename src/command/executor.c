#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../random/random.h"
#include "../memory/memory.h"
#include "../datatype/datatype.h"
#include "../result/result.h"
#include "../printer/printer.h"
#include "../datatype/stringtype.h"
#include "../traindata/traindata.h"
#include "../network/config.h"
#include "../network/activator.h"
#include "../network/bias.h"
#include "../network/loss.h"
#include "../network/vector.h"
#include "../network/matrix.h"
#include "../network/layer.h"
#include "../network/network.h"

#include "command.h"
#include "executor.h"

void loadConfigExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    Result *result = loadNetworkConfig(parameter->getValue(parameter));
    if (!result->success(result)) {
        printMessage(RED, result->message);
        return;
    }

    NetworkConfig *config = (NetworkConfig*)result->getData(result);
    NeuralNetwork *network = getNeuralNetwork();
    if (network != NULL) {
        releaseNeuralNetwork(network);
    }

    bool success = constructNeuralNetwork(config);
    if (success) {
        printMessage(WHITE, "Neural network has been constructed and initialized successfully^+^");
    } else {
        printMessage(RED, "Neural network construction encounters error, please check logs^o^");
    }
}

void loadModelExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void saveModelExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void showModelExecutor(Command *command) {
    String *name = command->name;

    printf("execute command[name:%s]\n", name->getValue(name));
}

void loadMnistDataExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void loadMnistLabelExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;

    printf("execute command[name:%s]\n", name->getValue(name));
}

void startTrainExecutor(Command *command) {
    String *name = command->name;

    printf("execute command[name:%s]\n", name->getValue(name));
}

void predictExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;

    printf("execute command[name:%s]\n", name->getValue(name));
}

void showHelpExecutor(Command *command) {
    showCommandInfo();
}

void printMemExecutor(Command *command) {
    printAllocationTable();
}

void quitExecutor(Command *command) {
    printMessage(WHITE, "Hello Bye Bye^+^\n");
    exit(0);
}

bool loadConfigRequireConfirm(Command *command) {
    NeuralNetwork *network = getNeuralNetwork();
    if (network != NULL) {
        printMessage(YELLOW, "Do you really want to load model configuration from file[Yes|No]??");
        return true;
    } else {
        return false;
    }
}

bool loadModelRequireConfirm(Command *command) {
    printMessage(YELLOW, "Do you really want to load model from file[Yes|No]??");
    return true;
}

bool saveModelRequireConfirm(Command *command) {
    printMessage(YELLOW, "Do you really want to save model to file[Yes|No]??");
    return true;
}

bool startTrainRequireConfirm(Command *command) {
    printMessage(YELLOW, "Do you really want to start model training[Yes|No]??");
    return true;
}

bool quitRequireConfirm(Command *command) {
    printMessage(YELLOW, "Do you really want to quit[Yes|No]??");
    return true;
}