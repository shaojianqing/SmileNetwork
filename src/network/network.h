typedef struct Network Network;

struct Network {

    Layer *layers;

    int layerCount;

    void (*addLayer)(Network *this, Layer *layer);

    Result* (*train)(Network *this, TrainData **tainDataList, int trainDataCount);

    Result* (*predict)(Network *this, Vector *vector);
};

Network* createNetwork();