#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "logger.h"

#define LOG_BUFFER_SIZE     256

#define DEBUG              "[DEBUG]"
#define INFO               "[INFO]"
#define WARN               "[WARN]"
#define ERROR              "[ERROR]"

Logger logger;

FILE *logFile = NULL;

char *logFilepath = "~/logs/SmileNetwork.log";

static void debug(const char *format, ...);

static void info(const char *format, ...);

static void warn(const char *format, ...);

static void error(const char *format, ...);

static void log(const char *level, const char *message);

void initLoggerConfig() {

    logFile = fopen(logFilepath, "a");
    
    logger.debug = debug;
    logger.info = info;
    logger.warn = warn;
    logger.error = error;
}

static void debug(const char *format, ...) {
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    log(DEBUG, buffer);
}

static void info(const char *format, ...) {
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    log(INFO, buffer);
}

static void warn(const char *format, ...) {
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    log(WARN, buffer);
}

static void error(const char *format, ...) {
    char buffer[LOG_BUFFER_SIZE];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    log(ERROR, buffer);
}

static void log(const char *level, const char *message) {
    long now = time(NULL);
	char dateBuffer[32];
	strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d %H:%M:%S", localtime(&now));

    printf("%s %s %s.\n", dateBuffer, level, message);
}