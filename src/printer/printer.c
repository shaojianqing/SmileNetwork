#include <stdio.h>
#include <stdarg.h>

#include "printer.h"

#define PRINTER_BUFFER_SIZE     256

void printMessage(Color color, const char *format, ...) {

    char buffer[PRINTER_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    if (color == RED) {
        printf("\033[31m%s\033[0m", buffer);
    } else if (color == BLACK) {
        printf("\033[30m%s\033[0m", buffer);
    } else if (color == GREEN) {
        printf("\033[32m%s\033[0m", buffer);
    } else if (color == YELLOW) {
        printf("\033[33m%s\033[0m", buffer);
    } else if (color == BLUE) {
        printf("\033[34m%s\033[0m", buffer);
    } else if (color == CYAN) {
        printf("\033[36m%s\033[0m", buffer);
    } else if (color == PURPLE) {
        printf("\033[35m%s\033[0m", buffer);
    } else if (color == WHITE) {
        printf("\033[37m%s\033[0m", buffer);
    }
}

