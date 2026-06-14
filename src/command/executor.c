#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../random/random.h"
#include "../memory/memory.h"
#include "../logger/logger.h"
#include "../datatype/datatype.h"
#include "../result/result.h"
#include "../printer/printer.h"
#include "../datatype/stringtype.h"
#include "../dataset/mnist.h"
#include "../dataset/train.h"
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

extern Logger logger;

void loadConfigExecutor(Command *command) {
    String *name = getCommandName(command);
    String *parameter = getCommandParam(command);
    
    Result *result = loadNetworkConfig(parameter->getValue(parameter));
    if (!success(result)) {
        printMessage(RED, getMessage(result));
        releaseResult(result);
        return;
    }

    NetworkConfig *config = (NetworkConfig*)getData(result);
    releaseResult(result);

    NeuralNetwork *network = getNeuralNetwork();
    if (network != NULL) {
        releaseNeuralNetwork(network);
    }

    bool success = constructNeuralNetwork(config);
    releaseNetworkConfig(config);
    if (success) {
        printMessage(WHITE, "Neural network has been constructed and initialized successfully^+^");
    } else {
        printMessage(RED, "Neural network construction encounters error, please check logs^o^");
    }
}

void loadModelExecutor(Command *command) {
    String *name = getCommandName(command);
    String *parameter = getCommandParam(command);

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
    logger.info("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void saveModelExecutor(Command *command) {
    String *name = getCommandName(command);
    String *parameter = getCommandParam(command);

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void showModelExecutor(Command *command) {
    String *name = getCommandName(command);

    printf("execute command[name:%s]\n", name->getValue(name));
    logger.info("execute command[name:%s]", name->getValue(name));
}

void loadMnistDataExecutor(Command *command) {
    String *name = getCommandName(command);
    String *parameter = getCommandParam(command);

    bool success = loadMnistDataFromFile(parameter->getValue(parameter));
    if (success) {
        printMessage(WHITE, "Load mnist data successfully^+^");
    } else {
        printMessage(RED, "Load mnist data error, please check log for detail^o^");
    }
}

void loadMnistLabelExecutor(Command *command) {
    String *name = getCommandName(command);
    String *parameter = getCommandParam(command);

    bool success = loadMnistLabelFromFile(parameter->getValue(parameter));
    if (success) {
        printMessage(WHITE, "Load mnist label successfully^+^");
    } else {
        printMessage(RED, "Load mnist label error, please check log for detail^o^");
    }
}

void startTrainExecutor(Command *command) {
    NeuralNetwork *neuralNetwork = getNeuralNetwork();
    if (neuralNetwork == NULL) {
        printMessage(RED, "Neural network has not been initialized, please initialize network firstly^o^");
        return;
    }

    int epoch = 0, trainEpochCount = getTrainEpochCount(neuralNetwork);
    while (epoch < trainEpochCount) { 
        int trainBatchSize = getTrainBatchSize(neuralNetwork);
        Result *loadResult = loadTrainBatchStochastic(trainBatchSize);
        if (!success(loadResult)) {
            printMessage(RED, getMessage(loadResult));
            releaseResult(loadResult);
            return;
        }

        TrainBatch *trainBatch = (TrainBatch*)getData(loadResult);
        releaseResult(loadResult);

        Result *trainResult = train(neuralNetwork, trainBatch, epoch);
        if (success(trainResult)) {
            printMessage(WHITE, "Neural network has trained data with [epoch:%i]^+^", epoch);
        } else {
            printMessage(RED, getMessage(trainResult));
        }
        releaseResult(trainResult);
        releaseTrainBatch(trainBatch);

        epoch++;
    }
    printMessage(WHITE, "Neural network has finished training in [epoch:%i]^+^", epoch);
}

void predictExecutor(Command *command) {
    String *name = getCommandName(command);
    String *parameter = getCommandParam(command);

    printf("execute command[name:%s]\n", name->getValue(name));
}

void validateExecutor(Command *command) {
    NeuralNetwork *neuralNetwork = getNeuralNetwork();
    if (neuralNetwork == NULL) {
        printMessage(RED, "Neural network has not been initialized, please initialize network firstly^o^");
        return;
    }

    Result *loadResult = loadTrainBatchForValidate();
    if (!success(loadResult)) {
        printMessage(RED, getMessage(loadResult));
        releaseResult(loadResult);
        return;
    }

    TrainBatch *trainBatch = (TrainBatch*)getData(loadResult);
    releaseResult(loadResult);

    Result *validateResult = validate(neuralNetwork, trainBatch);
     if (success(validateResult)) {
        printMessage(WHITE, "Neural network has been validate successfully^+^");
    } else {
        printMessage(RED, getMessage(validateResult));
    }
    releaseResult(validateResult);
    releaseTrainBatch(trainBatch);
}

void showHelpExecutor(Command *command) {
    showCommandInfo();
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

bool loadMnistDataRequireConfirm(Command *command) {
    MnistData *mnistData = getMnistTrainData();
    if (mnistData == NULL) {
        printMessage(YELLOW, "Do you really want to load mnist train data from file[Yes|No]??");
    } else {
        printMessage(YELLOW, "Do you really want to reload mnist train data and replace the current data instance[Yes|No]??");
    }
    return true;
}

bool loadMnistLabelRequireConfirm(Command *command) {
    MnistLabel *mnistLabel = getMnistTrainLabel();
    if (mnistLabel == NULL) {
        printMessage(YELLOW, "Do you really want to load mnist train label from file[Yes|No]??");
    } else {
        printMessage(YELLOW, "Do you really want to reload mnist train label and replace the current label instance[Yes|No]??");
    }
    return true;
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

bool validateRequireConfirm(Command *command) {
    printMessage(YELLOW, "Do you really want to validate model[Yes|No]??");
    return true;   
}

bool quitRequireConfirm(Command *command) {
    printMessage(YELLOW, "Do you really want to quit[Yes|No]??");
    return true;
}