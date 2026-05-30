typedef struct Vector Vector;

typedef struct Label Label;

typedef struct TrainData TrainData;

struct TrainData {

    Vector *data;

    Label *label;
};

TrainData *createTrainData(Vector *data, Label *label);