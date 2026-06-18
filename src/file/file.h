typedef struct File File;

File* openFile(const char *filepath, int flag);

void closeFile(File *file);

long long getFileSize(File *this);

char* readCharString(File *this);

byte* readByteBuffer(File *this);

bool writeByteBuffer(File *this, byte* buffer, long count);
