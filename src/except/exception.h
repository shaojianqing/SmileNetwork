#include <setjmp.h>

typedef jmp_buf Context;

typedef struct Exception Exception;

typedef struct StackFrame StackFrame;

typedef enum ExceptionState ExceptionState;

enum ExceptionState {
    NORMAL = 0,
    EXCEPTION = 1
};

struct StackFrame {

   Context context;

   Exception *exception;
};

struct Exception {

    char *type;

    char *message;

    char *filename;

    int line;
};

#define try                                                 \
            volatile int exceptionState;                    \
            StackFrame frameContext;                        \
            StackFrame *stackFrame = &frameContext;         \
            pushStackFrame(stackFrame);                     \
            exceptionState = setjmp(stackFrame->context);   \
            if (exceptionState == NORMAL) 

#define catch(e) else if (isTypeOfException(e, stackFrame->exception)) 

#define uncaught else

#define fetchException() stackFrame->exception

#define end                                                 \
        if (exceptionState == NORMAL) {                     \
           popStackFrame();                                 \
        }                                                           
                    
#define throw(e, m) throwException(e, m, __FILE__, __LINE__)

#define initException(e, m)                                \
    Exception exception = {"", m, 0};                      \
    e = &exception;                                                                                                     

#define RuntimeExceptionType                               "RuntimeException"
#define ValidationExceptionType                            "ValidationException"
#define MemoryAllocExceptionType                           "MemoryAllocException"
#define NullPointerExceptionType                           "NullPointerException"
#define DataIllegalExceptionType                           "DataIllegalException"
#define FileOperateExceptionType                           "FileOperateException"
#define ConfigErrorExceptionType                           "ConfigErrorException"

void initExceptionStack();

bool isTypeOfException(char *type, Exception *exception);

void throwException(Exception *exception, char *message, char *file, int line);

void pushStackFrame(StackFrame *stackFrame);

StackFrame* popStackFrame();