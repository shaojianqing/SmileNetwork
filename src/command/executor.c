#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "../common/common.h"
#include "../datatype/datatype.h"
#include "../datatype/stringtype.h"

#include "command.h"
#include "executor.h"

void loadConfigExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    command->release(command);

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));

    
}

void loadModelExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    command->release(command);

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void saveModelExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    command->release(command);

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void showModelExecutor(Command *command) {
    String *name = command->name;
    command->release(command);

    printf("execute command[name:%s]\n", name->getValue(name));
}

void loadMnistDataExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    command->release(command);

    printf("execute command[name:%s, parameter:%s]\n", name->getValue(name), parameter->getValue(parameter));
}

void loadMnistLabelExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    command->release(command);

    printf("execute command[name:%s]\n", name->getValue(name));
}

void startTrainExecutor(Command *command) {
    String *name = command->name;
    command->release(command);

    printf("execute command[name:%s]\n", name->getValue(name));
}

void predictExecutor(Command *command) {
    String *name = command->name;
    String *parameter = command->parameter;
    command->release(command);

    printf("execute command[name:%s]\n", name->getValue(name));
}

void showHelpExecutor(Command *command) {
    command->release(command);
    showCommandInfo();
}

void quitExecutor(Command *command) {
    command->release(command);
    printf("Hello Bye Bye^+^\n");
    exit(0);
}