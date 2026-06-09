typedef struct Vector Vector;

typedef struct Activator Activator;

typedef enum ActivatorKind ActivatorKind;

enum ActivatorKind {
    SIGMOID = 1,
    SOFTMAX = 2,
    EQUAL = 3,
    RELU = 4
};

struct Activator {

    Result* (*activate)(Vector *vector);

    Result* (*derivative)(Vector *vector);
};

void initActivatorMap();

Activator* getActivator(ActivatorKind kind);