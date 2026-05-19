
typedef enum LossKind LossKind;

enum LossKind {
    MSE = 1,
    CEL = 2
};

typedef float (*Loss)(Vector *source, Vector *target);

Loss getLoss(LossKind kind);