typedef struct Vector Vector;

typedef struct Label Label;

typedef struct TrainData TrainData;

struct TrainData {

    Vector *data;

    Label *label;

    void (*retain)(TrainData *this);

    void (*release)(TrainData *this);
};

TrainData *createTrainData(Vector *data, Label *label);