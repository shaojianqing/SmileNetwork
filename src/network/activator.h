typedef struct Vector Vector;

typedef struct Activator Activator;

typedef enum ActivatorKind ActivatorKind;

enum ActivatorKind {
    SIGMOID = 1,
    SOFTMAX = 2,
    EQUAL = 3,
    RELU = 3
};

struct Activator {

    Vector* (*activate)(Vector *vector);

    Vector* (*derivative)(Vector *vector);
};

void initActivatorMap();

Activator* getActivator(ActivatorKind kind);