
#include <list>

struct twoBools {
	bool nextBit;
	bool isCharFullyRead;
};

class BitManager {
public:
	BitManager();
	~BitManager();

	//Methods used in saving
	bool addBit(bool bit);	
	unsigned char getCurrentChar();

	//Methods used in loading
	void setChar(unsigned char charToAdd);
	twoBools getNextBit();

private:
	
	unsigned char byte;
	int counter;
	void setBit(int position,bool bit);
	void setTrue(int position);
	int getBit(int position);

};