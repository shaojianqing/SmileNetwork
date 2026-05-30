#include <memory.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../result/result.h"
#include "../logger/logger.h"

#include "file.h"

extern Logger logger;

static Result* readCharString(File *this);

static Result* readByteBuffer(File *this);

static Result* writeByteBuffer(File *this, byte* buffer);

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

    File *file = (File*)malloc(sizeof(File));
    if (file == NULL) {
        close(fd);
        logger.error("fail to create file instance for memory allocation error^o^");
        return NULL;
    }

    file->fd = fd;
    file->size = fileStat.st_size;
    file->readCharString = readCharString;
    file->readByteBuffer = readByteBuffer;
    file->writeByteBuffer = writeByteBuffer;

    return file;
}

void closeFile(File *file) {
    if (file != NULL) {
        close(file->fd);
        free(file);
    }
}

static Result* readCharString(File *this) {
    if (this == NULL) {
        char *message = "file instance is null for reading char string operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

     int readSize = this->size;
    char *buffer = (char *)malloc(readSize + 1);
    if (buffer == NULL) {
        char *message = "fail to read char string for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOCATE_ERROR, message);
    }

    int count = 0;
    while (count < readSize) {
       int num = read(this->fd, (buffer + count), (readSize - count));
       if (num == 0) {
            break;
       } else if (num < 0) {
            free(buffer);
            char *message = "fail to read char string for file operation error^o^";
            return createResultWithoutData(FILE_READ_ERROR, message);
       }
       count += num;
    }

    buffer[readSize] = '\0';
    return createResultWithData(SUCCESS, NULL, TYPE_CHAR_BUFFER, buffer);
}

static Result* readByteBuffer(File *this) {
        if (this == NULL) {
        char *message = "file instance is null for reading byte buffer operation^o^";
        return createResultWithoutData(INSTANCE_IS_NULL, message);
    }

     int readSize = this->size;
    byte *buffer = (byte *)malloc(readSize);
    if (buffer == NULL) {
        char *message = "fail to read byte buffer for memory allocation error^o^";
        return createResultWithoutData(MEMORY_ALLOCATE_ERROR, message);
    }

    int count = 0;
    while (count < readSize) {
       int num = read(this->fd, (buffer + count), (readSize - count));
       if (num == 0) {
            break;
       } else if (num < 0) {
            free(buffer);
            char *message = "fail to read byte buffer for file operation error^o^";
            return createResultWithoutData(FILE_READ_ERROR, message);
       }
       count += num;
    }
    
    return createResultWithData(SUCCESS, NULL, TYPE_BYTE_BUFFER, buffer);
}

static Result* writeByteBuffer(File *this, byte* buffer) {
    return NULL;
}