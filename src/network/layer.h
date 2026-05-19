typedef struct Layer Layer;

struct Layer {

    Matrix *matrix;

    Activator activator;

    int reference;

    Result* (*input)(Layer *this, Vector *vector);

    Result* (*output)(Layer *this, Vector *vector);

    void (*retain)(Layer *this);

    void (*release)(Layer *this);
};

Layer *createLayer(Matrix *matrix, Activator activator);