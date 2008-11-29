#include "SmileyEngine.h"
#include "Math.h"

extern SMH *smh;

BitStream::BitStream() {
	isOpen = false;
}

BitStream::~BitStream() { 
	if (isOpen) throw new System::Exception("Error: Attempting to delete the BitStream while a stream is still open.");
}


/**
 * Opens an input/output stream to the specified file. The mode parameter should be
 * FILE_WRITE or FILE_READ, indicating how to open the stream.
 */
void BitStream::open(std::string fileName, int _mode) {
	
	if (isOpen) throw new System::Exception("Error: Attempting to open a stream that is already open.");

	isOpen = true;
	mode = _mode;
	counter = 0;
	byte = 0;
	numRead = numWritten = 0;

	if (mode == FILE_READ) {
		inFile.open(fileName.c_str(), std::ios::binary);
	} else {
		outFile.open(fileName.c_str(), std::ios::binary);
		outString = "";
	}
}

/**
 * Closes the stream. If it is an output stream, the data in the ouput buffer will be
 * written at this time.
 */
void BitStream::close() {

	if (!isOpen) throw new System::Exception("Error: Attemping to close a stream that isn't open.");
	isOpen = false;

	if (mode == FILE_READ) {
		inFile.close();
	} else if (mode == FILE_WRITE) {
		while (!writeBit(false)); // fill out the last byte if necessary
		outFile.write(outString.c_str(), outString.length());
		outFile.close();
	}
}

/**
 * Writes a single bit to the output stream. Returns true if this bit was the last
 * bit in a byte.
 */
bool BitStream::writeBit(bool bit) {
	
	if (mode != FILE_WRITE) throw new System::Exception("Error: attempting to write to a stream that is opened in read mode!");

	if (bit) {	
		byte = byte | (unsigned char)pow(2, 7-counter); // | is the bitwise OR
	} 

	counter++;
	numWritten++;
	if (counter > 7) {
		//End of this bit - add it to the output string and start a new byte
		outString += byte;
		counter = 0;
		byte = 0;
		return true;
	} else {
		return false;
	}
}

/**
 * Reads a single bit from the input stream.
 */
bool BitStream::readBit() {

	if (mode != FILE_READ) throw new System::Exception("Error: Attemping to read from a stream that is opened in write mode!");

	if (counter == 0) {
		//We are at the beginning of a new byte, so read one in from the input file
		char buffer[1];
		inFile.read(buffer,1);
		byte = (unsigned char)buffer[0];
	}

	bool bit = byte & (unsigned char)pow(2,7-counter);
	numRead++;

	counter++;
	if (counter > 7) {
		counter=0;
	}

	return bit;
}

/**
 * Writes a byte to the output stream. The byte should be passed in as an integer from 0-255.
 */
void BitStream::writeByte(int byte) {

	if (mode != FILE_WRITE) throw new System::Exception("Error: attempting to write to a file that is opened in read mode!");
	
	for (int i = 7; i >= 0; i--) {
		if (byte >= pow(2, i)) {
			writeBit(true);
			byte -= pow(2, i);
		} else {
			writeBit(false);
		}
	}
}

/** 
 * Reads a byte from the input stream. The byte will be returned as an integer from 0-255.
 */
int BitStream::readByte() {

	if (mode != FILE_READ) throw new System::Exception("Error: Attemping to read from a stream that is opened in write mode!");

	int byte = 0;
	for (int i = 7; i >= 0; i--) {
		if (readBit()) {
			byte += pow(2, i);
		}
	}
	return byte;
}

/**
 * Returns the number of bits that have been written since the stream was opened.
 */ 
int BitStream::getNumBitsWritten() {
	return numWritten;
}

/**
 * Returns the number of bits that have been read since the stream was opened.
 */
int BitStream::getNumBitsRead() {
	return numRead;
}

/**
 * Static test method.
 */
void BitStream::test() {
		
	BitStream *b = new BitStream();

	//Bits test
	smh->hge->System_Log("---Testing bits---");
	b->open("test.txt", FILE_WRITE);
	b->writeBit(true);
	b->writeBit(false);
	b->writeBit(true);
	b->writeBit(true);
	b->writeBit(false);
	b->writeBit(false);
	b->writeBit(true);
	b->writeBit(false);
	//----
	b->writeBit(true);
	b->writeBit(false);
	b->writeBit(true);
	b->writeBit(true);
	b->writeBit(false);
	b->writeBit(true);
	b->close();
	b->open("test.txt", FILE_READ);
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("----");
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	b->close();

	//Bytes test
	smh->hge->System_Log("---Testing bytes---");
	b->open("test.txt", FILE_WRITE);
	b->writeByte(248);
	b->close();
	//---
	b->open("test.txt", FILE_READ);
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	smh->hge->System_Log("%d", b->readBit());
	b->close();
	//---
	b->open("test.txt", FILE_WRITE);
	b->writeByte(128);
	b->writeByte(230);
	b->close();
	//---
	b->open("test.txt", FILE_READ);
	smh->hge->System_Log("%d", b->readByte());
	smh->hge->System_Log("%d", b->readByte());
	b->close();

	//Bits and bytes test
	smh->hge->System_Log("---Testing bits and bytes---");
	b->open("test.txt", FILE_WRITE);
	b->writeBit(true);
	b->writeBit(false);
	b->writeBit(true);
	b->writeByte(43);
	b->writeBit(true);
	b->writeByte(82);
	b->close();
	b->open("test.txt", FILE_READ);
	smh->hge->System_Log("true: %d", b->readBit());
	smh->hge->System_Log("false: %d", b->readBit());
	smh->hge->System_Log("true: %d", b->readBit());
	smh->hge->System_Log("43: %d", b->readByte());
	smh->hge->System_Log("true: %d", b->readBit());
	smh->hge->System_Log("82: %d", b->readByte());
	b->close();
	
	delete b;
}
