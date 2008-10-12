#include "BitManager.h"
#include "Math.h"

BitManager::BitManager() {
	counter=0; //Start at position 0
	byte=0; //Fill the byte with 0's
}

BitManager::~BitManager() { }

/*
 * This function adds a bit to the bit string. The current position in the bit string
 * is tracked, and when a new bit is added, that value is set to the bit argument.
 * If the unsigned char is full, return true.
 */
bool BitManager::addBit(bool bit) {

	if (bit) {	
		byte = byte | (unsigned char)pow(2, 7-counter); // | is the bitwise OR
	} 

	counter++;
	return (counter > 7);
}

unsigned char BitManager::getCurrentChar() {
	unsigned char returnByte = byte;
    
	//clear out the byte and reset the counter
	byte=0;
	counter=0;

	return returnByte;
}

// Methods used in loading ///////////////////
void BitManager::setChar(unsigned char charToAdd) {
	byte = charToAdd;
	counter = 0;
}

twoBools BitManager::getNextBit() {
	twoBools returnData;
	
	returnData.nextBit = getBit(7-counter);
	
	returnData.isCharFullyRead = false;
	counter++;
	if (counter > 7) {
		counter=0;
		returnData.isCharFullyRead = true;
	}

	return returnData;

}

// Private ////////////////////////////////////////////////////

int BitManager::getBit(int position) {
	//pow(2,8-position) makes a byte of: 10000000 for position 0, 01000000 for position 1, etc.
	//& is the bitwise "and".
	//Our byte    (ex 01100111) 
	// & our pow  (ex 00000100) will return non-zero if the bit at the position in our byte was 1
	//              = 00000100
	return byte & (unsigned char)pow(2,position);
}