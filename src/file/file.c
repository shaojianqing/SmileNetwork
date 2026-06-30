#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"
#include "../except/exception.h"
#include "../except/assertion.h"
#include "../logger/logger.h"

#include "file.h"

struct File {

    int fd;

    long long size;
};

extern Logger logger;

static Exception MemoryAllocException = {MemoryAllocExceptionType};
static Exception FileOperateException = {FileOperateExceptionType};

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

char* readCharString(File *this) {

    assertNotNull(this, "file instance is null for reading char string operation!");

    long readSize = this->size;
    char *string = (char *)allocate((int)readSize + 1);
    if (string == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for reading string buffer creation.");
    }

    long count = 0;
    while (count < readSize) {
        long num = read(this->fd, (string + count), (readSize - count));
        if (num == 0) {
            break;
        } else if (num < 0) {
            release(string);
            throw(&FileOperateException, "fail to read char string for file operation error.");
        }
        count += num;
    }

    string[readSize] = '\0';
    return string;
}

byte* readByteBuffer(File *this) {

    assertNotNull(this, "file instance is null for reading byte buffer operation!");

    long readSize = this->size;
    byte *buffer = (byte *)allocate((int)readSize);
    if (buffer == NULL) {
        throw(&MemoryAllocException, "can not allocate memory for reading byte buffer creation.");
    }

    long count = 0;
    while (count < readSize) {
        long num = read(this->fd, (buffer + count), (readSize - count));
        if (num == 0) {
            break;
        } else if (num < 0) {
            release(buffer);
            throw(&FileOperateException, "fail to read byte buffer for file operation error.");
        }
        count += num;
    }
    
    return buffer;
}

bool writeByteBuffer(File *this, byte* buffer, long count) {
    return false;
}
