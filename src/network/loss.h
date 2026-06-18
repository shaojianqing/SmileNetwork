typedef enum ActivatorLossKind ActivatorLossKind;

enum ActivatorLossKind {
    EQUAL_MSE = 1,
    SOFTMAX_CEL = 2
};

typedef float (*ActivatorLossFunc)(Vector *predict, Vector *expect);

typedef Vector* (*ActivatorGradientFunc)(Vector *predict, Vector *expect);

ActivatorLossFunc getActivatorLossFunc(ActivatorLossKind kind);

ActivatorGradientFunc getActivatorGradientFunc(ActivatorLossKind kind);

Activator* getActivatorByActivatorLossKind(ActivatorLossKind kind);