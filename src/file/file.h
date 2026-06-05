typedef struct File File;

struct File {
    int fd;

    long long size;

    long long (*getFileSize)(File *this);

    Result* (*readCharString)(File *this);

    Result* (*readByteBuffer)(File *this);

    Result* (*writeByteBuffer)(File *this, byte* buffer, long count);
};

File* openFile(const char *filepath, int flag);

void closeFile(File *file);
