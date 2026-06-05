
typedef struct ByteReader ByteReader;

struct ByteReader {

    byte* buffer;

	long length;

    long position;

	byte (*readByte)(ByteReader *this);

	short (*readShort)(ByteReader *this);

	int (*readInteger)(ByteReader *this);

	void (*setPosition)(ByteReader *this, long position);
};

ByteReader *createByteReader(byte* code, long length, long position);