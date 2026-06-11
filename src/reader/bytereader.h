
typedef struct ByteReader ByteReader;

ByteReader *createByteReader(byte* code, long length, long position);

void releaseByteReader(ByteReader *byteReader);

byte readByte(ByteReader *this);

short readShort(ByteReader *this);

int readInteger(ByteReader *this);

void setPosition(ByteReader *this, long position);
