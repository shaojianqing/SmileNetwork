#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../datatype/datatype.h"
#include "../datatype/stringtype.h"
#include "../datatype/hashmap.h"
#include "../printer/printer.h"

#include "command.h"
#include "executor.h"

#define INITIAL_COMMAND_COUNT           10
#define COMMAND_BUFFER_SIZE             256
#define COMMAND_NAME_BUFFER_SIZE        100
#define COMMAND_PARAM_BUFFER_SIZE       100

#define CONFIRM_STATE_UNKNOWN           0
#define CONFIRM_STATE_YES               1
#define CONFIRM_STATE_NO                2

#define CONFIRM_STRING_YES              "Yes"
#define CONFIRM_STRING_NO               "No"

#define LOAD_CONFIG_NAME                "loadConfig"
#define LOAD_MODEL_NAME                 "loadModel"
#define SAVE_MODEL_NAME                 "saveModel"
#define SHOW_MODEL_NAME                 "showModel"
#define LOAD_MNIST_DATA_NAME            "loadMnistData"
#define LOAD_MNIST_LABEL_NAME           "loadMnistLabel"
#define START_TRAIN_NAME                "startTrain"
#define PREDICT_NAME                    "predict"
#define VALIDATE_NAME                   "validate"
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
#define VALIDATE_DESC                   "validate"
#define SHOW_HELP_DESC                  "showHelp"
#define QUIT_DESC                       "quit"

typedef struct Configuration Configuration;

struct Configuration {

    String *name;

    String *description;

    bool requireParameter;

    Executor executor;

    RequireConfirm requireConfirm;
};

struct Command {

    String *name;

    String *parameter;

    Executor execute;

    RequireConfirm requireConfirm;
};

static Map *commandConfigMap;

static char commandBuffer[COMMAND_BUFFER_SIZE];

static char commandName[COMMAND_NAME_BUFFER_SIZE];

static char commandParam[COMMAND_PARAM_BUFFER_SIZE];

static void resetDataBuffer(char *databuffer, int size);

static bool isCommandBlank(char *commandLine);

static Command* parseCommand(char *commandLine);

static bool defaultRequireConfirm(Command *command);

static bool parseCommandElement(char *commandLine, char *commandElement);

static Command *buildCommand(String *name, String *parameter, Executor executor, RequireConfirm requireConfirm);

static Configuration* buildConfiguration(String *name, String *description, 
                                         bool requireParameter, Executor executor, RequireConfirm requireConfirm);

void initCommandConfig() {
    commandConfigMap = createHashMap(StringHashCode, StringEqualFun, INITIAL_COMMAND_COUNT);

    String *loadConfigName = createString(LOAD_CONFIG_NAME);
    String *loadConfigDesc = createString(LOAD_CONFIG_DESC);
    Configuration *loadConfigConfiguration = buildConfiguration(loadConfigName, loadConfigDesc, true, loadConfigExecutor, loadConfigRequireConfirm);
    commandConfigMap->put(commandConfigMap, loadConfigName, loadConfigConfiguration);

    String *loadModelName = createString(LOAD_MODEL_NAME);
    String *loadModelDesc = createString(LOAD_MODEL_DESC);
    Configuration *loadModelConfiguration = buildConfiguration(loadModelName, loadModelDesc, true, loadModelExecutor, loadModelRequireConfirm);
    commandConfigMap->put(commandConfigMap, loadModelName, loadModelConfiguration);

    String *saveModelName = createString(SAVE_MODEL_NAME);
    String *saveModelDesc = createString(SAVE_MODEL_DESC);
    Configuration *saveModelConfiguration = buildConfiguration(saveModelName, saveModelDesc, true, saveModelExecutor, saveModelRequireConfirm);
    commandConfigMap->put(commandConfigMap, saveModelName, saveModelConfiguration);

    String *showModelName = createString(SHOW_MODEL_NAME);
    String *showModelDesc = createString(SHOW_MODEL_DESC);
    Configuration *showModelConfiguration = buildConfiguration(showModelName, showModelDesc, false, showModelExecutor, defaultRequireConfirm);
    commandConfigMap->put(commandConfigMap, showModelName, showModelConfiguration);

    String *loadMnistDataName = createString(LOAD_MNIST_DATA_NAME);
    String *loadMnistDataDesc = createString(LOAD_MNIST_DATA_DESC);
    Configuration *loadMnistDataConfiguration = buildConfiguration(loadMnistDataName, loadMnistDataDesc, true, loadMnistDataExecutor, loadMnistDataRequireConfirm);
    commandConfigMap->put(commandConfigMap, loadMnistDataName, loadMnistDataConfiguration);

    String *loadMnistLabelName = createString(LOAD_MNIST_LABEL_NAME);
    String *loadMnistLabelDesc = createString(LOAD_MNIST_LABEL_DESC);
    Configuration *loadMnistLabelConfiguration = buildConfiguration(loadMnistLabelName, loadMnistLabelDesc, true, loadMnistLabelExecutor, loadMnistLabelRequireConfirm);
    commandConfigMap->put(commandConfigMap, loadMnistLabelName, loadMnistLabelConfiguration);

    String *startTrainName = createString(START_TRAIN_NAME);
    String *startTrainDesc = createString(START_TRAIN_DESC);
    Configuration *startTrainConfiguration = buildConfiguration(startTrainName, startTrainDesc, false, startTrainExecutor, startTrainRequireConfirm);
    commandConfigMap->put(commandConfigMap, startTrainName, startTrainConfiguration);

    String *validateName = createString(VALIDATE_NAME);
    String *validateDesc = createString(VALIDATE_DESC);
    Configuration *validateConfiguration = buildConfiguration(validateName, validateDesc, false, validateExecutor, validateRequireConfirm);
    commandConfigMap->put(commandConfigMap, validateName, validateConfiguration);

    String *predictName = createString(PREDICT_NAME);
    String *predictDesc = createString(PREDICT_DESC);
    Configuration *predictConfiguration = buildConfiguration(predictName, predictDesc, true, predictExecutor, defaultRequireConfirm);
    commandConfigMap->put(commandConfigMap, predictName, predictConfiguration);

    String *showHelpName = createString(SHOW_HELP_NAME);
    String *showHelpDesc = createString(SHOW_HELP_DESC);
    Configuration *showHelpConfiguration = buildConfiguration(showHelpName, showHelpDesc, false, showHelpExecutor, defaultRequireConfirm);
    commandConfigMap->put(commandConfigMap, showHelpName, showHelpConfiguration);

    String *quitName = createString(QUIT_NAME);
    String *quitDesc = createString(QUIT_DESC);
    Configuration *quitConfiguration = buildConfiguration(quitName, quitDesc, false, quitExecutor, quitRequireConfirm);
    commandConfigMap->put(commandConfigMap, quitName, quitConfiguration);
}

static Command* parseCommand(char *commandLine) {
    if (isCommandBlank(commandLine)) {
        return NULL;
    }

    resetDataBuffer(commandName, COMMAND_NAME_BUFFER_SIZE);
    bool success = parseCommandElement(commandLine, commandName);
    if (!success) {
        printMessage(RED, "Command name is illegal or invalid^o^");
        return NULL;
    }
    String *name = createString(commandName);
    if (commandConfigMap->containsKey(commandConfigMap, name)) {
        Configuration *configuration = commandConfigMap->get(commandConfigMap, name);
        if (configuration->requireParameter) {
            resetDataBuffer(commandParam, COMMAND_PARAM_BUFFER_SIZE);
            bool success = parseCommandElement(commandLine, commandParam);
            if (!success) {
                printMessage(RED, "Command parameter is blank[%s]^o^", name->getValue(name));
                return NULL;
            }
            
            String *parameter = createString(commandParam);
            return buildCommand(name, parameter, configuration->executor, configuration->requireConfirm);
        }
        return buildCommand(name, NULL, configuration->executor, configuration->requireConfirm);
    } else {
        printMessage(RED, "Command is not supported[%s]^o^", name->getValue(name));
        return NULL;
    }
}

static int parseConfirmState(char *commandLine) {
    if (strcasecmp(commandLine, CONFIRM_STRING_YES) == 0) {
        return CONFIRM_STATE_YES;
    } else if (strcasecmp(commandLine, CONFIRM_STRING_NO) == 0) {
        return CONFIRM_STATE_NO;
    } else {
        return CONFIRM_STATE_UNKNOWN;
    }
}

static Configuration* buildConfiguration(String *name, String *description, 
                                         bool requireParameter, Executor executor, RequireConfirm requireConfirm) {
    Configuration *configuration = (Configuration *)allocate(sizeof(Configuration));
    if (configuration != NULL) {
        configuration->name = name;
        configuration->description = description;
        configuration->requireParameter = requireParameter;
        configuration->requireConfirm = requireConfirm;
        configuration->executor = executor;
    }
    return configuration;
}

static Command *buildCommand(String *name, String *parameter, Executor executor, RequireConfirm requireConfirm) {
    Command *command = (Command *)allocate(sizeof(Command));
    if (command != NULL) {
        command->name = name;
        command->parameter = parameter;
        command->requireConfirm = requireConfirm;
        command->execute = executor;
        
        return command;
    }
    return NULL;
}

static void releaseCommand(Command *command) {
    if (command != NULL) {
        releaseString(command->name);
        releaseString(command->parameter);
        release(command);
    }
}

void runCommandEvent() {
    Command *currentCommand = NULL;
    bool commandRequireConfirm = false;
    while(true) {
        if (!commandRequireConfirm) {
            printMessage(GREEN, "Please enter command here^+^");
        }
        resetDataBuffer(commandBuffer, COMMAND_BUFFER_SIZE);
        fgets(commandBuffer, COMMAND_BUFFER_SIZE, stdin);
        commandBuffer[strcspn(commandBuffer, "\n")] = 0;
        if (commandRequireConfirm) {
            int state = parseConfirmState(commandBuffer);
            if (state == CONFIRM_STATE_UNKNOWN) {
                printMessage(YELLOW, "Please enter confirm decision[Yes|No]!");
                continue;
            }

            if (state == CONFIRM_STATE_YES) {
                currentCommand->execute(currentCommand);
                releaseCommand(currentCommand);
                commandRequireConfirm = false;
                currentCommand = NULL;
                
            }
            
            if (state == CONFIRM_STATE_NO) {
                releaseCommand(currentCommand);
                commandRequireConfirm = false;
                currentCommand = NULL;
            }
        } else {
            currentCommand = parseCommand(commandBuffer);
            if (currentCommand != NULL) {
                if (currentCommand->requireConfirm(currentCommand)) {
                    commandRequireConfirm = true;
                } else {
                    currentCommand->execute(currentCommand);
                    releaseCommand(currentCommand);
                    currentCommand = NULL;
                }
            }
        }
    }
}

String* getCommandName(Command *this) {
    if (this != NULL) {
        return this->name;
    }
    return NULL;
}

String* getCommandParam(Command *this) {
    if (this != NULL) {
        return this->parameter;
    }
    return NULL;
}

void showNetworkInfo() {
    printMessage(YELLOW, "\n");
    printMessage(YELLOW, "  ***  **   ** **** **    *****   *   ** ***** **    ** *****        *      **      *   ** ***** ****** **         **  *****  *****   *** **   ");
    printMessage(YELLOW, " ** ** *******  **  **    **      **  ** **    **    ** **   **     ***     **      **  ** **    ****** **         ** **   ** **   ** ** **    ");
    printMessage(YELLOW, " **    ** * **  **  **    ****    *** ** ****  **    ** **   **    ** **    **      *** ** ****    **    **   *    ** **   ** **   ** ****     ");
    printMessage(YELLOW, "   **  **   **  **  **    ****    ** *** ****  **    ** *****     **   **   **      ** *** ****    **     ** *** **   **   ** *****   ** **    ");
    printMessage(YELLOW, " ** ** **   **  **  ***** **      **  ** **    **    ** ** **    *********  *****   **  ** **      **      *** ***    **   ** ** **   **  **   ");
    printMessage(YELLOW, "  ***  **   ** **** ***** *****   **   * *****   ****   **  *** ***     *** *****   **   * *****   **       ** **      *****  **  *** ***  **  ");
    printMessage(YELLOW, "\n");
    
    printMessage(WHITE, "+---------------------------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ SmileNetwork is deep neural network implementation with C language. It implemnets the MLP architecture and could be configured as both      +");
    printMessage(WHITE, "+ multi-layer deep neural network or convolution neural network for image recognition usage. Please refer to the link below for more detail:  +");
    printMessage(WHITE, "+ https://github.com/shaojianqing/SmileNetwork [Enter showHelp for command help info]                                                         +");
    printMessage(WHITE, "+---------------------------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "\n");
}

void showCommandInfo() {
    printMessage(WHITE, "\n");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ Command           + Description                                                                                                             +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ loadConfig        + load model configuration file from config path parameter. It is used to initialize model.                               +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ loadModel         + load model data file from mdoel path parameter. It is used to load model saved before.                                  +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ saveModel         + save model data to mdoel file with path parameter. It is used to save model after trained.                              +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ showModel         + show model structure with layer and dimension. It is used to show model configurations.                                 +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ loadMnistData     + load mnist train data from mnist data file path parameter. Support mnist-fashion dataset.                               +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ loadMnistLabel    + load mnist train label from mnist label file path parameter. Support mnist-fashion dataset.                             +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ startTrain        + start to train model with loaded data and label. It is called after train data is loaded.                               +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ validate          + validate model with model data and model label. It could be called only after trained.                                  +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ predict           + predict model label with model data by the trained model. It is called only after trained.                              +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ showHelp          + show command help information list just as above content.                                                               +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "+ quit              + quit running process without saving model data to model file anyway.                                                    +");
    printMessage(WHITE, "+-------------------+-------------------------------------------------------------------------------------------------------------------------+");
    printMessage(WHITE, "\n");
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
    commandElement[j] = '\0';
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

static bool defaultRequireConfirm(Command *command) {
    return false;
}

static void resetDataBuffer(char *databuffer, int size) {
    for (int i=0;i<size;++i) {
        databuffer[i] = '\0';
    }
}