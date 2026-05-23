typedef struct Logger Logger;

struct Logger {

    void (*info)(const char *format, ...);

    void (*warn)(const char *format, ...);

    void (*error)(const char *format, ...);
};

void initLoggerConfig();