typedef struct Logger Logger;

typedef void (*LogFunc)(const char *format, ...);

struct Logger {

    LogFunc debug;

    LogFunc info;

    LogFunc warn;

    LogFunc error;
};

void initLoggerConfig();