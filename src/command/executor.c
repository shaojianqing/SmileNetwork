#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../logger/logger.h"
#include "../except/exception.h"
#include "../datatype/datatype.h"
#include "../printer/printer.h"
#include "../datatype/stringtype.h"
#include "../dataset/mnist.h"
#include "../dataset/train.h"
#include "../network/config.h"
#include "../network/activator.h"
#include "../generator/generator.h"
#include "../network/bias.h"
#include "../network/loss.h"
#include "../network/vector.h"
#include "../network/matrix.h"
#include "../network/linear.h"
#include "../network/network.h"

#include "command.h"
#include "executor.h"

extern Logger logger;

void loadConfigExecutor(Command *command) {
    try {
        String *name = getCommandName(command);
        String *parameter = getCommandParam(command);
        
        DeepNetworkConfig *config = loadNetworkConfig(parameter->getValue(parameter));
        DeepNeuralNetwork *network = getDeepNeuralNetwork();
        if (network != NULL) {
            releaseDeepNeuralNetwork(network);
        }

        bool success = constructDeepNeuralNetwork(config);
        releaseDeepNetworkConfig(config);
        if (success) {
            printMessage(WHITE, "Neural network has been constructed and initialized successfully^+^");
        } else {
            printMessage(RED, "Neural network construction encounters error, please check logs^o^");
        }
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("load network config raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void loadModelExecutor(Command *command) {
    try {
        String *name = getCommandName(command);
        String *parameter = getCommandParam(command);

        printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
        logger.info("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("load network model raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void saveModelExecutor(Command *command) {
    try {
        String *name = getCommandName(command);
        String *parameter = getCommandParam(command);

        printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("save network model raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void showModelExecutor(Command *command) {
    try {
        String *name = getCommandName(command);

        printf("execute command[name:%s]\n", name->getValue(name));
        logger.info("execute command[name:%s]", name->getValue(name));
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("show network model raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void loadMnistDataExecutor(Command *command) {
    try {
        String *name = getCommandName(command);
        String *parameter = getCommandParam(command);

        bool success = loadMnistDataFromFile(parameter->getValue(parameter));
        if (success) {
            printMessage(WHITE, "Load mnist data successfully^+^");
        } else {
            printMessage(RED, "Load mnist data error, please check log for detail^o^");
        }
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("load mnist data raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void loadMnistLabelExecutor(Command *command) {
    try {
        String *name = getCommandName(command);
        String *parameter = getCommandParam(command);

        bool success = loadMnistLabelFromFile(parameter->getValue(parameter));
        if (success) {
            printMessage(WHITE, "Load mnist label successfully^+^");
        } else {
            printMessage(RED, "Load mnist label error, please check log for detail^o^");
        }
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("load mnist label raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void startTrainExecutor(Command *command) {
    try {
        DeepNeuralNetwork *neuralNetwork = getDeepNeuralNetwork();
        if (neuralNetwork == NULL) {
            printMessage(RED, "Neural network has not been initialized, please initialize network firstly^o^");
            return;
        }

        int epoch = 0, trainEpochCount = getTrainEpochCount(neuralNetwork);
        while (epoch < trainEpochCount) { 
            int trainBatchSize = getTrainBatchSize(neuralNetwork);
            TrainBatch *trainBatch = loadTrainBatchStochastic(trainBatchSize);
            bool success = train(neuralNetwork, trainBatch, epoch);
            if (success) {
                printMessage(WHITE, "Neural network has trained data with [epoch:%i]^+^", epoch);
            }

            releaseTrainBatch(trainBatch);
            epoch++;
        }
        printMessage(WHITE, "Neural network has finished training in [epoch:%i]^+^", epoch);
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("train model raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void predictExecutor(Command *command) {
    try {
        String *name = getCommandName(command);
        String *parameter = getCommandParam(command);

        printf("execute command[name:%s]\n", name->getValue(name));
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("predict model raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void validateExecutor(Command *command) {
    try {
        DeepNeuralNetwork *neuralNetwork = getDeepNeuralNetwork();
        if (neuralNetwork == NULL) {
            printMessage(RED, "Neural network has not been initialized, please initialize network firstly^o^");
            return;
        }

        TrainBatch *validationBatch = loadTrainBatchForValidate();
        bool success = validate(neuralNetwork, validationBatch);
        if (success) {
            printMessage(WHITE, "Neural network has been validate successfully^+^");
        }

        releaseTrainBatch(validationBatch);
    } uncaught {
        Exception *ex = fetchException();
        printMessage(RED, ex->message);
        logger.error("validate model raise exception[type:%s,file:%s,line:%d,message:%s]", 
            ex->type, ex->filename, ex->line, ex->message);
    } end
}

void showHelpExecutor(Command *command) {
    showCommandInfo();
}

void quitExecutor(Command *command) {
    printMessage(WHITE, "Hello Bye Bye^+^\n");
    exit(0);
}

bool loadConfigRequireConfirm(Command *command) {
    DeepNeuralNetwork *network = getDeepNeuralNetwork();
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