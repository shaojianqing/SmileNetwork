typedef struct Command Command;

typedef void (*Executor)(Command *command);

typedef bool (*RequireConfirm)(Command *command);

struct Command {

    String *name;

    String *parameter;

    Executor execute;

    RequireConfirm requireConfirm;
};

void initCommandConfig();

void showNetworkInfo();

void showCommandInfo();

void runCommandEvent();