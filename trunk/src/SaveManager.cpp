#include "SaveManager.h"

#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>

#include "Environment.h"
#include "hge.h"
#include "smiley.h"
#include "ChangeManager.h"
#include "BitManager.h"

extern Environment *theEnvironment;
extern HGE *hge;

extern float timePlayed;
extern float gameStart;

/**
 * Constructor
 */ 
SaveManager::SaveManager() {
	currentSave = -1;
	changeManager = new ChangeManager();
	bitManager = new BitManager();
	resetCurrentData();
	loadFileInfo();
}

/**
 * Destructor
 */
SaveManager::~SaveManager() {
	delete changeManager;
	delete bitManager;
}

/**
 * Returns whether or not (gridX, gridY) is explored in the current area
 */
bool SaveManager::isExplored(int gridX, int gridY) {
	return explored[currentArea][gridX][gridY];
}

/**
 * Marks the screen that smiley can see as explored
 */ 
void SaveManager::explore(int gridX, int gridY) {
	for (int curGridY = gridY - 6; curGridY <= gridY + 6; curGridY++) {
		for (int curGridX = gridX - 8; curGridX <= gridX + 8; curGridX++) {
			if (inBounds(curGridX,curGridY)) {
				explored[currentArea][curGridX][curGridY] = true;
			}
		}
	}
}

/**
 * Resets the save data that is currently in memory.
 */
void SaveManager::resetCurrentData() {

	changeManager->reset();

	for (int i = 0; i < NUM_BOSSES; i++) killedBoss[i] = false;
	
	if (currentSave == 3) { //make it a REAL GAME
		for (int i = 0; i < NUM_ABILITIES; i++) hasAbility[i] = false;

		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 4; j++) {
				numKeys[i][j] = 0;
			}
		}
	} else { //Start with keys and abilities
		for (int i = 0; i < NUM_ABILITIES; i++) hasAbility[i] = true;
	
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 4; j++) {
				numKeys[i][j] = 5;
			}
		}
	}
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			numGems[i][j] = 0;
		}
	}

	//reset explored data
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++) {
				explored[i][j][k] = false;
			}
		}
	}

	money = 0;
	for (int i = 0; i < 3; i++) numUpgrades[i] = 0;
	currentArea = FOUNTAIN_AREA;
	timePlayed = 0;
	playerGridX = 0;
	playerGridY = 0;
	currentHint = 0;
}

/**
 * Loads save data from a file into memory.
 */
void SaveManager::load(int fileNumber) {

	hge->System_Log("Loading save file %d", fileNumber);

	changeManager->reset();

	currentSave = fileNumber;

	//Select the specified save file
	std::ifstream inFile;
	char buffer[1];
	char twoBuffer[2];
	char threeBuffer[3];
	if (currentSave == 0) inFile.open("Data/Save/file1.sav");
	else if (currentSave == 1) inFile.open("Data/Save/file2.sav");
	else if (currentSave == 2) inFile.open("Data/Save/file3.sav");
	else if (currentSave == 3) inFile.open("Data/Save/file4.sav");
	
	//Load time played
	timePlayed = files[fileNumber].timePlayed;

	//Load abilties
	for (int i = 0; i < NUM_ABILITIES; i++) {
		inFile.read(buffer,1);
		hasAbility[i] = (atoi(buffer) == 1);
	}

	//Load keys
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			inFile.read(buffer,1);
			numKeys[i][j] = atoi(buffer);
		}
	}

	//Load gems
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			inFile.read(buffer,1);
			numGems[i][j] = atoi(buffer);
		}
	}

	//Load money
	inFile.read(threeBuffer,3);
	money = atoi(threeBuffer);

	//Load upgrades
	for (int i = 0; i < 3; i++) {
		inFile.read(twoBuffer,2);
		numUpgrades[i] = atoi(twoBuffer);
	}

	//Load which bosses have been slain
	for (int i = 0; i < NUM_BOSSES; i++) {
		inFile.read(buffer,1);
		killedBoss[i] = (atoi(buffer) == 0);		
	}

	//Load player zone and location
	inFile.read(buffer,1);
	currentArea = atoi(buffer);
	inFile.read(threeBuffer,3);
	playerGridX = atoi(threeBuffer);
	inFile.read(threeBuffer,3);
	playerGridY = atoi(threeBuffer);

	//Load current hint
	inFile.read(twoBuffer, 2);
	currentHint = atoi(twoBuffer);

	//Load changed shit
	inFile.read(threeBuffer, 3);
	int numChanges = atoi(threeBuffer);
	int area, x, y;
	for (int i = 0; i < numChanges; i++) {
		
		//Read area
		inFile.read(twoBuffer, 2);
		area = atoi(twoBuffer);

		//Read x coordinate
		inFile.read(threeBuffer, 3);
		x = atoi(threeBuffer);

		//Read y coordinate
		inFile.read(threeBuffer, 3);
		y = atoi(threeBuffer);

		changeManager->change(area, x, y);

	}

	//Load exploration data
	twoBools nextTwoBools;
	unsigned char nextChar;

	inFile.read(buffer,1); //newline
	inFile.read(buffer,1); //newline

	inFile.read(buffer,1);
	nextChar = (unsigned char)buffer[0];
	bitManager->setChar(nextChar);

	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				nextTwoBools = bitManager->getNextBit();
				explored[i][j][k] = nextTwoBools.nextBit;
				if (nextTwoBools.isCharFullyRead) {
					inFile.read(buffer,1);
					nextChar = (unsigned char)buffer[0];
					bitManager->setChar(nextChar);
				}			
			}
			//inFile.read(buffer,1); //newline
		}
		//inFile.read(buffer,1); //newline
	}

}

/**
 * Saves the current save data in memory to a file.
 */
void SaveManager::save() {

	

	hge->System_Log("Saving file %d", currentSave);

	std::ofstream exFile;
	std::string exString;
	std::string numberString;

	//Select the specified save file
	if (currentSave == 0) {
		exFile.open("Data/Save/file1.sav");
	} else if (currentSave == 1) {
		exFile.open("Data/Save/file2.sav");
	} else if (currentSave == 2) {
		exFile.open("Data/Save/file3.sav");
	} else if (currentSave == 3) {
		exFile.open("Data/Save/file4.sav");
	}

	

	//Abilities
	for (int i = 0; i < NUM_ABILITIES; i++) exString += (hasAbility[i] ? "1" : "0");

	//Keys
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			exString += intToString(numKeys[i][j]);
		}
	}

	//Gems
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			exString += intToString(numGems[i][j]);
		}
	}

	//Money (3 digits)
	if (money < 10) { 
		numberString = "00";
		numberString += intToString(money);
	} else if (money < 100) {
		numberString = "0";
		numberString += intToString(money);
	} else {
		numberString = intToString(money);
	}
	exString += numberString.c_str();

	//Upgrades (2 digits)
	for (int i = 0; i < 3; i++) {
		if (numUpgrades[i] < 10) {
			numberString = "0";
			numberString += intToString(numUpgrades[i]);
			exString += numberString.c_str();
		} else {
			exString += intToString(numUpgrades[i]);
		}
	}

	//Bosses
	for (int i = 0; i < NUM_BOSSES; i++) exString += (killedBoss[i] ? "0" : "1");

	//Zone
	exString += intToString(currentArea);

	//X-coord
	if (playerGridX < 10) { 
		numberString = "00";
		numberString += intToString(playerGridX);
	} else if (playerGridX < 100) {
		numberString = "0";
		numberString += intToString(playerGridX);
	} else {
		numberString = intToString(playerGridX);
	}
	exString += numberString.c_str();
	
	//Y-coord
	if (playerGridY < 10) { 
		numberString = "00";
		numberString += intToString(playerGridY);
	} else if (playerGridY < 100) {
		numberString = "0";
		numberString += intToString(playerGridY);
	} else {
		numberString = intToString(playerGridY);
	}
	exString += numberString.c_str();

	//Current hint
	if (currentHint < 10) { 
		numberString = "0";
		numberString += intToString(currentHint);
	} else {
		numberString = intToString(currentHint);
	}
	exString += numberString.c_str();

	//Changed shit
	exString += changeManager->toString();

	unsigned char nextCharToWrite;

	//Exploration data
	exString += "\n\n";
		
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				if (bitManager->addBit(explored[i][j][k])) { //if true, it means the char is full
					nextCharToWrite = bitManager->getCurrentChar();
					exString += nextCharToWrite;
				}				
			}
			//exString += "\n";
		}
//		exString += "\n";
	}

	// Now, we write one more char. Even if we didn't fill the char yet,
	// we still don't want to lose any exploration data.
	nextCharToWrite = bitManager->getCurrentChar();
	exString += nextCharToWrite;

	// Write one more empty char, just to be safe. This is necessary in case we use up
	// exactly the right amount of chars in loading, since it automatically starts
	// looking at the next one.
	nextCharToWrite = 0;
	exString += nextCharToWrite;

	exString += "\n";

	//Write the string to the save file
	exFile.write(exString.c_str(), exString.length());
	exString.clear();
	exString = "";

	//Close the file!
	exFile.close();
	
}

/**
 * Starts a new save file in the specified slot
 */
void SaveManager::startNewGame(int fileNumber) {

	hge->System_Log("Creating new save in file %d", fileNumber);

	files[fileNumber].empty = false;
	files[fileNumber].timePlayed = 0;
	files[fileNumber].completion = 0;
	
	currentSave = fileNumber;
	
	saveFileInfo();
	resetCurrentData();
	save();

}

/**
 * This doesn't actually delete anything. The information in the header file
 * is just reset.
 */
void SaveManager::deleteFile(int file) {

	hge->System_Log("Deleting file %d", file);

	files[file].empty = true;
	files[file].timePlayed = 0;
	files[file].completion = 0;

	saveFileInfo();

}

//////////////////// SAVE HEADER (Data/Save/info.dat) SHIT /////////////////////////

/**
 * Saves file info to Data/Save/info.dat
 */
void SaveManager::saveFileInfo() {

	std::ofstream infoFile;
	std::string infoString;
	std::string numberString;

	infoFile.open("Data/Save/info.dat");
	infoString = "";

	//Build a string with all the file header info
	for (int i = 0; i < 4; i++) {

		//Whether or not the file is empty
		infoString += files[i].empty ? "1" : "0";
		
		//Time played (6 digits) I wish C strings didn't suck dick
		if (files[i].timePlayed < 10) {
			numberString = "00000";
			numberString += intToString(files[i].timePlayed);
		} else if (files[i].timePlayed < 100) {
			numberString = "0000";
			numberString += intToString(files[i].timePlayed);
		} else if (files[i].timePlayed < 1000) {
			numberString = "000";
			numberString += intToString(files[i].timePlayed);
		} else if (files[i].timePlayed < 10000) {
			numberString = "00";
			numberString += intToString(files[i].timePlayed);
		} else if (files[i].timePlayed < 100000) { 
			numberString = "0";
			numberString += intToString(files[i].timePlayed);		
		} else {
			numberString = intToString(files[i].timePlayed);
		}
		infoString += numberString.c_str();

		//Completion percentage
		if (files[i].completion < 10) {
			numberString = "0";
			numberString += intToString(files[i].completion);
		} else {
			numberString = intToString(files[i].completion);
		}
		infoString += numberString.c_str();

	}

	//Write the data
	infoFile.write(infoString.c_str(), infoString.length());
	infoFile.close();

}

/**
 * Loads save file info from Data/Save/info.dat
 */ 
void SaveManager::loadFileInfo() {

	std::ifstream inFile;
	char oneBuffer[1];
	char twoBuffer[2];
	char sixBuffer[6];
	inFile.open("Data/Save/info.dat");

	//If the info file doesn't exist, create a new one with empty files
	if (!inFile.good()) {
		for (int i = 0; i < 4; i++) {
			files[i].empty = true;
			files[i].timePlayed = 0;
			files[i].completion = 0;
		}
		saveFileInfo();
	} else {

		//Read file info
		for (int i = 0; i < 4; i++) {

			//Empty file
			inFile.read(oneBuffer,1);
			files[i].empty = (atoi(oneBuffer) == 1);

			//Time played
			inFile.read(sixBuffer,6);
			files[i].timePlayed = atoi(sixBuffer);

			//Completion
			inFile.read(twoBuffer,2);
			files[i].completion = atoi(twoBuffer);

		}

	}

}

/**
 * Returns whether or not the specified save file is empty.
 */
bool SaveManager::isFileEmpty(int file) {
	return files[file].empty;
}

/**
 * Returns the time played for the specified save file.
 */
int SaveManager::getTimePlayed(int file) {
	return files[file].timePlayed;
}

/**
 * Returns the completion % of the specified save file.
 */
int SaveManager::getCompletion(int file) {
	return files[file].completion;
}

/** 
 * Adds the specified time to the timePlayed for the specified save file.
 */
void SaveManager::incrementTimePlayed(int file, int amount) {
	files[file].timePlayed += amount;
}