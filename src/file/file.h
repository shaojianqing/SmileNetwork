typedef struct File File;

File* openFile(const char *filepath, int flag);

void closeFile(File *file);

long long getFileSize(File *this);

Result* readCharString(File *this);

Result* readByteBuffer(File *this);

Result* writeByteBuffer(File *this, byte* buffer, long count);
