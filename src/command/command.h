typedef struct Command Command;

typedef void (*Executor)(Command *command);

typedef void (*Release)(Command *this);

struct Command {

    String *name;

    String *parameter;

    Executor execute;

    Release release;
};

void initCommandConfig();

void showNetworkInfo();

void showCommandInfo();

void runCommandEvent();