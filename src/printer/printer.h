
typedef enum Color Color;

enum Color {
    RED = 1,
    BLACK = 2,
    GREEN = 3,
    YELLOW = 4,
    BLUE = 5,
    CYAN = 6,
    PURPLE = 7, 
    WHITE = 8,
};

void printMessage(Color color, const char *format, ...);