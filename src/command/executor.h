void loadConfigExecutor(Command *command);

void loadModelExecutor(Command *command);

void saveModelExecutor(Command *command);

void showModelExecutor(Command *command);

void loadMnistDataExecutor(Command *command);

void loadMnistLabelExecutor(Command *command);

void startTrainExecutor(Command *command);

void predictExecutor(Command *command);

void showHelpExecutor(Command *command);

void printMemExecutor(Command *command);

void quitExecutor(Command *command);


bool loadConfigRequireConfirm(Command *command);

bool loadModelRequireConfirm(Command *command);

bool loadMnistDataRequireConfirm(Command *command);

bool loadMnistLabelRequireConfirm(Command *command);

bool saveModelRequireConfirm(Command *command);

bool startTrainRequireConfirm(Command *command);

bool quitRequireConfirm(Command *command);