typedef struct File File;

struct File {
    int fd;

    long size;

    Result* (*readCharString)(File *this);

    Result* (*readByteBuffer)(File *this);

    Result* (*writeByteBuffer)(File *this, byte* buffer);
};

File* openFile(const char *filepath, int flag);

void closeFile(File *file);
