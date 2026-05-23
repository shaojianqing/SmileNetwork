#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../datatype/datatype.h"
#include "../datatype/stringtype.h"

#include "command.h"
#include "executor.h"

void loadConfigExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
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

void quitExecutor(Command *command) {
    printf("Hello Bye Bye^+^\n");
    exit(0);
}

bool loadConfigRequireConfirm(Command *command) {
    printf("\033[33mDo you really want to load model configuration from file[Yes|No]??\n\033[0m");
    return true;
}

bool loadModelRequireConfirm(Command *command) {
    printf("\033[33mDo you really want to load model from file[Yes|No]??\n\033[0m");
    return true;
}

bool saveModelRequireConfirm(Command *command) {
    printf("\033[33mDo you really want to save model to file[Yes|No]??\n\033[0m");
    return true;
}

bool startTrainRequireConfirm(Command *command) {
    printf("\033[33mDo you really want to start model training[Yes|No]??\n\033[0m");
    return true;
}

bool quitRequireConfirm(Command *command) {
    printf("\033[33mDo you really want to quit[Yes|No]??\n\033[0m");
    return true;
}