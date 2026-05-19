typedef struct Label Label;

struct Label {

    void (*retain)(Label *this);

    void (*release)(Label *this);
};

Label *createLabel();