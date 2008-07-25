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

extern Environment *theEnvironment;
extern HGE *hge;

extern float timePlayed;
extern float gameStart;

/**
 * Constructor
 */ 
SaveManager::SaveManager() {
	currentSave = -1;
	resetCurrentData();
	loadFileInfo();
}

/**
 * Destructor
 */
SaveManager::~SaveManager() {

}

/**
 * Returns whether or not (gridX, gridY) is explored in the current area
 */
bool SaveManager::isExplored(int gridX, int gridY) {
	return explored[currentArea][(gridX - (gridX%8))/8][(gridY - (gridY%8))/8];
}

/**
 * Marks the 8x8 square that Smiley is currently in as explored
 */ 
void SaveManager::explore(int gridX, int gridY) {
	for (int curGridY=gridY-4;curGridY<=gridY+4;curGridY+=4) {
		for (int curGridX=gridX-4;curGridX<=gridX+4;curGridX+=4) {
			if (inBounds(curGridX,curGridY)) {
				explored[currentArea][(curGridX-curGridX%8)/8][(curGridY-curGridY%8)/8] = true;
			}			
		}
	}
}

/**
 * Resets the save data that is currently in memory.
 */
void SaveManager::resetCurrentData() {
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
	money = 0;
	for (int i = 0; i < 3; i++) numUpgrades[i] = 0;
	for (int i = 0; i < 999; i++) {
		collectedItem[i] = false;
		openedDoor[i] = false;
		cylinderChanged[i] = false;
	}
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
		hge->System_Log("NINJA: %d", killedBoss[i]);
	}

	//Load player zone and location
	inFile.read(buffer,1);
	currentArea = atoi(buffer);
	inFile.read(threeBuffer,3);
	playerGridX = atoi(threeBuffer);
	inFile.read(threeBuffer,3);
	playerGridY = atoi(threeBuffer);

	//Load which items were collected and which doors are opened
	for (int i = 0; i < 999; i++) {
		inFile.read(buffer,1);
		collectedItem[i] = (atoi(buffer) == 1);
		inFile.read(buffer,1);
		openedDoor[i] = (atoi(buffer) == 1);
		inFile.read(buffer,1);
		cylinderChanged[i] = (atoi(buffer) == 1);
	}

	//Load current save
	inFile.read(twoBuffer, 2);
	currentHint = atoi(twoBuffer);

	//Load exploration data
	inFile.read(buffer,1); //newline
	inFile.read(buffer,1); //newline
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++) {
				inFile.read(buffer,1); //newline
				explored[i][j][k] = atoi(buffer) == 1;
			}
			inFile.read(buffer,1); //newline
		}
		inFile.read(buffer,1); //newline
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

	//Collected items and opened doors
	for (int i = 0; i < 999; i++) {
		exString += (collectedItem[i] ? "1" : "0");
		exString += (openedDoor[i] ? "1" : "0");
		exString += (cylinderChanged[i] ? "1" : "0");
	}

	//Current hint
	if (currentHint < 10) { 
		numberString = "0";
		numberString += intToString(currentHint);
	} else {
		numberString = intToString(currentHint);
	}
	exString += numberString.c_str();

	//Exploration data
	exString += "\n\n";
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++) {
				exString += intToString(explored[i][j][k]);
			}
			exString += "\n";
		}
		exString += "\n";
	}

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