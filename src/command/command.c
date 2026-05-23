#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../datatype/datatype.h"
#include "../datatype/stringtype.h"
#include "../datatype/hashmap.h"

#include "command.h"
#include "executor.h"

#define INITIAL_COMMAND_COUNT           10
#define COMMAND_BUFFER_SIZE             256
#define COMMAND_NAME_BUFFER_SIZE        100
#define COMMAND_PARAM_BUFFER_SIZE       100

#define LOAD_CONFIG_NAME                "loadConfig"
#define LOAD_MODEL_NAME                 "loadModel"
#define SAVE_MODEL_NAME                 "saveModel"
#define SHOW_MODEL_NAME                 "showModel"
#define LOAD_MNIST_DATA_NAME            "loadMnistData"
#define LOAD_MNIST_LABEL_NAME           "loadMnistLabel"
#define START_TRAIN_NAME                "startTrain"
#define PREDICT_NAME                    "predict"
#define SHOW_HELP_NAME                  "showHelp"
#define QUIT_NAME                       "quit"

#define LOAD_CONFIG_DESC                "loadConfig"
#define LOAD_MODEL_DESC                 "loadModel"
#define SAVE_MODEL_DESC                 "saveModel"
#define SHOW_MODEL_DESC                 "showModel"
#define LOAD_MNIST_DATA_DESC            "loadMnistData"
#define LOAD_MNIST_LABEL_DESC           "loadMnistLabel"
#define START_TRAIN_DESC                "startTrain"
#define PREDICT_DESC                    "predict"
#define SHOW_HELP_DESC                  "showHelp"
#define QUIT_DESC                       "quit"

typedef struct Configuration Configuration;

struct Configuration {

    String *name;

    String *description;

    bool requireParameter;

    Executor executor;
};

static char *commandBuffer;

static HashMap *commandConfigMap;

static void release(Command *this);

static bool isCommandBlank(char *commandLine);

static Command* parseCommand(char *commandLine);

static bool parseCommandElement(char *commandLine, char *commandElement);

static Command *buildCommand(String *name, String *parameter, Executor executor);

static Configuration* buildConfiguration(String *name, String *description, bool requireParameter, Executor executor);

void initCommandConfig() {
    commandConfigMap = createHashMap(StringHashCode, StringEqualFun, INITIAL_COMMAND_COUNT);

    String *loadConfigName = createString(LOAD_CONFIG_NAME);
    String *loadConfigDesc = createString(LOAD_CONFIG_DESC);
    Configuration *loadConfigConfiguration = buildConfiguration(loadConfigName, loadConfigDesc, true, loadConfigExecutor);
    commandConfigMap->put(commandConfigMap, loadConfigName, loadConfigConfiguration);

    String *loadModelName = createString(LOAD_MODEL_NAME);
    String *loadModelDesc = createString(LOAD_MODEL_DESC);
    Configuration *loadModelConfiguration = buildConfiguration(loadModelName, loadModelDesc, true, loadModelExecutor);
    commandConfigMap->put(commandConfigMap, loadModelName, loadModelConfiguration);

    String *saveModelName = createString(SAVE_MODEL_NAME);
    String *saveModelDesc = createString(SAVE_MODEL_DESC);
    Configuration *saveModelConfiguration = buildConfiguration(saveModelName, saveModelDesc, true, saveModelExecutor);
    commandConfigMap->put(commandConfigMap, saveModelName, saveModelConfiguration);

    String *showModelName = createString(SHOW_MODEL_NAME);
    String *showModelDesc = createString(SHOW_MODEL_DESC);
    Configuration *showModelConfiguration = buildConfiguration(showModelName, showModelDesc, false, showModelExecutor);
    commandConfigMap->put(commandConfigMap, showModelName, showModelConfiguration);

    String *loadMnistDataName = createString(LOAD_MNIST_DATA_NAME);
    String *loadMnistDataDesc = createString(LOAD_MNIST_DATA_DESC);
    Configuration *loadMnistDataConfiguration = buildConfiguration(loadMnistDataName, loadMnistDataDesc, true, loadMnistDataExecutor);
    commandConfigMap->put(commandConfigMap, loadMnistDataName, loadMnistDataConfiguration);

    String *loadMnistLabelName = createString(LOAD_MNIST_LABEL_NAME);
    String *loadMnistLabelDesc = createString(LOAD_MNIST_LABEL_DESC);
    Configuration *loadMnistLabelConfiguration = buildConfiguration(loadMnistLabelName, loadMnistLabelDesc, true, loadMnistLabelExecutor);
    commandConfigMap->put(commandConfigMap, loadMnistLabelName, loadMnistLabelConfiguration);

    String *startTrainName = createString(START_TRAIN_NAME);
    String *startTrainDesc = createString(START_TRAIN_DESC);
    Configuration *startTrainConfiguration = buildConfiguration(startTrainName, startTrainDesc, false, startTrainExecutor);
    commandConfigMap->put(commandConfigMap, startTrainName, startTrainConfiguration);

    String *predictName = createString(PREDICT_NAME);
    String *predictDesc = createString(PREDICT_DESC);
    Configuration *predictConfiguration = buildConfiguration(predictName, predictDesc, true, predictExecutor);
    commandConfigMap->put(commandConfigMap, predictName, predictConfiguration);

    String *showHelpName = createString(SHOW_HELP_NAME);
    String *showHelpDesc = createString(SHOW_HELP_DESC);
    Configuration *showHelpConfiguration = buildConfiguration(showHelpName, showHelpDesc, false, showHelpExecutor);
    commandConfigMap->put(commandConfigMap, showHelpName, showHelpConfiguration);

    String *quitName = createString(QUIT_NAME);
    String *quitDesc = createString(QUIT_DESC);
    Configuration *quitConfiguration = buildConfiguration(quitName, quitDesc, false, quitExecutor);
    commandConfigMap->put(commandConfigMap, quitName, quitConfiguration);
}

Command* parseCommand(char *commandLine) {
    if (isCommandBlank(commandLine)) {
        return NULL;
    }

    char *commandName = (char *)malloc(COMMAND_NAME_BUFFER_SIZE);
    bool success = parseCommandElement(commandLine, commandName);
    if (!success) {
        printf("Command name is illegal or invalid^o^\n");
        return NULL;
    }
    String *name = createString(commandName);
    free(commandName);

    if (commandConfigMap->containsKey(commandConfigMap, name)) {
        Configuration *configuration = commandConfigMap->get(commandConfigMap, name);
        if (configuration->requireParameter) {
            char *commandParam = (char *)malloc(COMMAND_PARAM_BUFFER_SIZE);
            bool success = parseCommandElement(commandLine, commandParam);
            if (!success) {
                printf("Command parameter is blank[%s]^o^\n", name->getValue(name));
                return NULL;
            }
            
            String *parameter = createString(commandParam);
            free(commandParam);

            return buildCommand(name, parameter, configuration->executor);
        }
        return buildCommand(name, NULL, configuration->executor);
    } else {
        printf("Command is not supported[%s]^o^\n", name->getValue(name));
        return NULL;
    }
}

static Configuration* buildConfiguration(String *name, String *description, bool requireParameter, Executor executor) {
    Configuration *configuration = (Configuration *)malloc(sizeof(Configuration));
    if (configuration != NULL) {
        configuration->name = name;
        configuration->description = description;
        configuration->requireParameter = requireParameter;
        configuration->executor = executor;
    }
    return configuration;
}

static Command *buildCommand(String *name, String *parameter, Executor executor) {
    Command *command = (Command *)malloc(sizeof(Command));
    if (command != NULL) {
        command->name = name;
        command->parameter = parameter;
        command->execute = executor;
        
        return command;
    }
    return NULL;
}

static void releaseCommand(Command *command) {
    if (command != NULL) {
        releaseString(command->name);
        releaseString(command->parameter);

        free(command);
    }
}

void runCommandEvent() {
    commandBuffer = (char *)malloc(COMMAND_BUFFER_SIZE);
    while(true) {
        printf("Please enter command here ^+^\n");
        fgets(commandBuffer, 200, stdin);
        commandBuffer[strcspn(commandBuffer, "\n")] = 0;
        Command *command = parseCommand(commandBuffer);
        if (command != NULL) {
            command->execute(command);
            releaseCommand(command);
        }
    }
}

void showNetworkInfo() {
    printf("\n\n");
    printf(" ****  **** **** ****     *  ** **** **  ** *****     *     **       *  ** **** **** **       **  ****  *****  ** ** \n");
    printf(" ** ** **   **   ** **    ** ** **   **  ** **  **   ***    **       ** ** **   **** **   *   ** **  ** **  ** ****  \n");
    printf(" ** ** **** **** ****     ***** **** **  ** *****   ** **   **       ***** ****  **   ** *** **  **  ** *****  ***   \n");
    printf(" ** ** **   **   **       ** ** **   **  ** ****   *******  *****    ** ** **    **    *** ***   **  ** ****   ****  \n");
    printf(" ****  **** **** **       **  * ****  ****  ** *****     ** *****    **  * ****  **     ** **     ****  ** *** ** ** \n");
    printf("\n\n");
    printf("+-------------------------------------------------------------------------------------------------------------------+\n");
    printf("+ SmileNetwork is deep neural network implementation with C language. It implemnets the MLP architecture and could  +\n");
    printf("+ be configured as multi-layer neural network for image recognition usage. Please refer to the link for detail:     +\n");
    printf("+ https://github.com/shaojianqing/SmileNetwork                                                                      +\n");
    printf("+-------------------------------------------------------------------------------------------------------------------+\n");
    printf("\n\n");
}

void showCommandInfo() {
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ Command           + Description                                                                                   +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ loadConfig        + load model configuration file from config path parameter. It is used to initialize model.     +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ loadModel         + load model data file from mdoel path parameter. It is used to load model saved before.        +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ saveModel         + save model data to mdoel file with path parameter. It is used to save model after trained.    +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ showModel         + show model structure with layer and dimension. It is used to show model configurations.       +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ loadMnistData     + load mnist train data from mnist data file path parameter. Support mnist-fashion dataset.     +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ loadMnistLabel    + load mnist train label from mnist label file path parameter. Support mnist-fashion dataset.   +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ startTrain        + start to train model with loaded data and label. It is called after train data is loaded.     +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ predict           + predict model label with model data by the trained model. It is called only after trained.    +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ showHelp          + show command help information list just as above content.                                     +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("+ quit              + quit running process without saving model data to model file anyway.                          +\n");
    printf("+-------------------+-----------------------------------------------------------------------------------------------+\n");
    printf("\n\n");
}

static bool parseCommandElement(char *commandLine, char *commandElement) {
    int i = 0, j = 0;
    bool parsed = false;
    while (commandLine[i] != '\0') {
        if (commandLine[i] == ' ') {
            if (parsed) {
                j++;
                commandElement[j] = '\0';
                return true;
                break;
            } else {
                i++;
            }
        } else {
            commandElement[j] = commandLine[i];
            commandLine[i] = ' ';
            parsed = true;
            i++;
            j++;
        }
    }
    return parsed;
}

static bool isCommandBlank(char *commandLine) {
    if (commandLine == NULL) {
        return true;
    }

    int i = 0;
    while(commandLine[i] != '\0') {
        if (commandLine[i] != ' ' && commandLine[i] != '\n' && commandLine[i] != '\r') {
            return false;
        }
        i++;
    }
    return true;
}