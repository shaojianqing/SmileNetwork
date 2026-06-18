#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../printer/printer.h"
#include "../datatype/stack.h"
#include "../logger/logger.h"

#include "exception.h"

#define INIT_CALL_STACK_CAPACITY 16

extern Logger logger;

Stack *callFrameStack;

void initExceptionStack() {
    callFrameStack = createStack(INIT_CALL_STACK_CAPACITY);
    if (callFrameStack == NULL) {
        printMessage(RED, "can not create call frame stack for memory allocation error!");
        exit(-1);
    }
}

bool isTypeOfException(char *type, Exception *exception) {
    if (exception == NULL) {
        return false;
    }

    if (strcasecmp(type, exception->type) == 0) {
        return true;
    }
    return false;
}

void pushStackFrame(StackFrame *stackFrame) {
    if (stackFrame != NULL) {
        push(callFrameStack, stackFrame);
    }
}

StackFrame* popStackFrame() {
    return pop(callFrameStack);
}

void throwException(Exception *exception, char *message, char *filename, int line) {
    StackFrame *stackFrame = (StackFrame*)pop(callFrameStack);
    if (stackFrame != NULL) {
        exception->message = message;
        exception->filename = filename;
        exception->line = line;

        stackFrame->exception = exception;

        longjmp(stackFrame->context, EXCEPTION);
    } else {
        logger.fatal("throw statement is not contained inside the try-catch context!");
        exit(-1);
    }
}