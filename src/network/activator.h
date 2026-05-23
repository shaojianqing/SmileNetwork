typedef struct Activator Activator;

typedef enum ActivatorKind ActivatorKind;

enum ActivatorKind {
    SIGMONID = 1,
    SOFTMAX = 2,
    RELU = 3
};

struct Activator {

    Vector* (*activate)(Vector *vector);

    Vector* (*derivative)(Vector *vector);
};

void initActivatorMap();

Activator* getActivator(ActivatorKind kind);