#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "../common/common.h"
#include "../common/constant.h"
#include "../memory/memory.h"

#include "bytereader.h"

static byte readByte(ByteReader *this);

static short readShort(ByteReader *this);

static int readInteger(ByteReader *this);

static void setPosition(ByteReader *this, long position);

ByteReader *createByteReader(byte* buffer, long length, long position) {
	if (buffer!=NULL && length>0) {
		ByteReader *byteReader = (ByteReader*)allocate(sizeof(ByteReader));
		if (byteReader!=NULL) {
			byteReader->buffer = buffer;
			byteReader->length = length;
			byteReader->position = position;

			byteReader->readByte = readByte;
			byteReader->readShort = readShort;
			byteReader->readInteger = readInteger;
            byteReader->setPosition = setPosition;
		}
		return byteReader;
	}
	return NULL;
}

static void setPosition(ByteReader *this, long position) {
	this->position = position;
}

static byte readByte(ByteReader *this) {
	byte data = this->buffer[this->position];
	this->position++;
	return data;
}

static short readShort(ByteReader *this) {
	byte data1 = readByte(this);
	byte data2 = readByte(this);
	return (data1 << 8) | data2;
}

static int readInteger(ByteReader *this) {
	byte data1 = readByte(this);
	byte data2 = readByte(this);
	byte data3 = readByte(this);
	byte data4 = readByte(this);
	return (data1 << 24) | (data2 << 16) | (data3 << 8) | data4;
}