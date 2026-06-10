typedef struct Command Command;

typedef void (*Executor)(Command *command);

typedef bool (*RequireConfirm)(Command *command);

void initCommandConfig();

void showNetworkInfo();

void showCommandInfo();

void runCommandEvent();

String* getCommandName(Command *this);

String* getCommandParam(Command *this);