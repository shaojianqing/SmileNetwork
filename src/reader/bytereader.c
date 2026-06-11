#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"

#include "bytereader.h"

struct ByteReader {

    byte* buffer;

	long length;

    long position;
};

ByteReader *createByteReader(byte* buffer, long length, long position) {
	if (buffer!=NULL && length>0) {
		ByteReader *byteReader = (ByteReader*)allocate(sizeof(ByteReader));
		if (byteReader!=NULL) {
			byteReader->buffer = buffer;
			byteReader->length = length;
			byteReader->position = position;
		}
		return byteReader;
	}
	return NULL;
}

void releaseByteReader(ByteReader *byteReader) {
	if (byteReader != NULL) {
		release(byteReader);
	}
}

void setPosition(ByteReader *this, long position) {
	this->position = position;
}

byte readByte(ByteReader *this) {
	byte data = this->buffer[this->position];
	this->position++;
	return data;
}

short readShort(ByteReader *this) {
	byte data1 = readByte(this);
	byte data2 = readByte(this);
	return (data1 << 8) | data2;
}

int readInteger(ByteReader *this) {
	byte data1 = readByte(this);
	byte data2 = readByte(this);
	byte data3 = readByte(this);
	byte data4 = readByte(this);
	return (data1 << 24) | (data2 << 16) | (data3 << 8) | data4;
}