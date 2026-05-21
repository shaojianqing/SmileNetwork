typedef struct Command Command;

typedef void (*Executor)(Command *command);

struct Command {

    String *name;

    String *parameter;

    Executor execute;
};

void initCommandConfig();

void showNetworkInfo();

void showCommandInfo();

void runCommandEvent();