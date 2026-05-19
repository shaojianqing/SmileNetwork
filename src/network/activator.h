typedef enum ActivatorKind ActivatorKind;

enum ActivatorKind {
    SIGMONID = 1,
    SOFTMAX = 2,
    RELU = 3
};

typedef Vector* (*Activator)(Vector *vector);

Activator getActivator(ActivatorKind kind);