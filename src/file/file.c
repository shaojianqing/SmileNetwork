#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../result/result.h"
#include "../logger/logger.h"

#include "file.h"

struct File {

    int fd;

    long long size;
};

extern Logger logger;

File* openFile(const char *filepath, int flags) {
    int fd = open(filepath, flags);
    if (fd < 0) {
        logger.error("fail to open file with path:[%s]^o^", filepath);
        return NULL;
    }

    struct stat fileStat;
    int code = stat(filepath, &fileStat);
    if (code < 0) {
        close(fd);
        logger.error("fail to get file stat with path:[%s]^o^", filepath);
        return NULL;
    }

    File *file = (File*)allocate(sizeof(File));
    if (file == NULL) {
        close(fd);
        logger.error("fail to create file instance for memory allocation error^o^");
        return NULL;
    }

    file->fd = fd;
    file->size = fileStat.st_size;

    return file;
}

void closeFile(File *file) {
    if (file != NULL) {
        close(file->fd);
        release(file);
    }
}

long long getFileSize(File *this) {
    if (this == NULL) {
        return 0;
    }
    return this->size;
}

Result* readCharString(File *this) {
    if (this == NULL) {
        char *message = "file instance is null for reading char string operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    long readSize = this->size;
    char *string = (char *)allocate((int)readSize + 1);
    if (string == NULL) {
        char *message = "fail to read char string for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    long count = 0;
    while (count < readSize) {
        long num = read(this->fd, (string + count), (readSize - count));
        if (num == 0) {
            break;
        } else if (num < 0) {
            release(string);
            char *message = "fail to read char string for file operation error^o^";
            return createResultWithoutData(FILE_READ_ERROR, message);
        }
        count += num;
    }

    string[readSize] = '\0';
    return createResultWithData(SUCCESS, NULL, TYPE_CHAR_BUFFER, string);
}

Result* readByteBuffer(File *this) {
        if (this == NULL) {
        char *message = "file instance is null for reading byte buffer operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

    long readSize = this->size;
    byte *buffer = (byte *)allocate((int)readSize);
    if (buffer == NULL) {
        char *message = "fail to read byte buffer for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOC_ERROR, message);
    }

    long count = 0;
    while (count < readSize) {
        long num = read(this->fd, (buffer + count), (readSize - count));
        if (num == 0) {
            break;
        } else if (num < 0) {
            release(buffer);
            char *message = "fail to read byte buffer for file operation error^o^";
            return createResultWithoutData(FILE_READ_ERROR, message);
        }
        count += num;
    }
    
    return createResultWithData(SUCCESS, NULL, TYPE_BYTE_BUFFER, buffer);
}

Result* writeByteBuffer(File *this, byte* buffer, long count) {
    return NULL;
}
