#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <sstream>
#include <iostream>

#include "SmileyEngine.h"
#include "Player.h"
#include "Environment.h"
#include "hge.h"
#include "smiley.h"
#include "ChangeManager.h"
#include "BitManager.h"
#include "boss.h"

extern SMH *smh;
extern HGE *hge;

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
 * Records a change at the specified tile.
 */
void SaveManager::change(int gridX, int gridY) {
	changeManager->change(currentArea, gridX, gridY);
}

/**
 * Returns whether or not the specified tile has a change on it.
 */
bool SaveManager::isTileChanged(int gridX, int gridY) {
	return changeManager->isChanged(currentArea, gridX, gridY);
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
			if (smh->environment->isInBounds(curGridX,curGridY)) {
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
	
	//make it a REAL GAME
	if (currentSave == 3) { 
		for (int i = 0; i < NUM_ABILITIES; i++) hasAbility[i] = false;
		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 4; j++) {
				numKeys[i][j] = 0;
			}
		}

	//Start with keys and abilities
	} else { 
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
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				explored[i][j][k] = false;
			}
		}
	}

	money = 0;
	tutorialManCompleted = false;
	for (int i = 0; i < 3; i++) numUpgrades[i] = 0;
	currentArea = FOUNTAIN_AREA;
	playerGridX = 0;
	playerGridY = 0;
	playerHealth = 5.0;
	playerMana = 100.0;

	timeFileLoaded = hge->Timer_GetTime();

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
	char fiveBuffer[5];
	if (currentSave == 0) inFile.open("Data/Save/file1.sav");
	else if (currentSave == 1) inFile.open("Data/Save/file2.sav");
	else if (currentSave == 2) inFile.open("Data/Save/file3.sav");
	else if (currentSave == 3) inFile.open("Data/Save/file4.sav");
	
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

	//Health and mana
	inFile.read(threeBuffer, 3);
	playerHealth = float(atoi(threeBuffer)) / 4.0;
	inFile.read(threeBuffer, 3);
	playerMana = float(atoi(threeBuffer));

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

	//Load Stats
	inFile.read(fiveBuffer, 5);
	numTongueLicks = atoi(fiveBuffer);
	inFile.read(fiveBuffer, 5);
	numEnemiesKilled = atoi(fiveBuffer);
	inFile.read(fiveBuffer, 5);
	damageDealt = atoi(fiveBuffer);
	inFile.read(fiveBuffer, 5);
	damageReceived = atoi(fiveBuffer);

	//Tutorial Man
	inFile.read(buffer, 1);
	tutorialManCompleted = atoi(buffer) == 1;

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
		}
	}

	timeFileLoaded = hge->Timer_GetTime();

}

/**
 * Saves the current save data in memory to a file.
 */
void SaveManager::save() {

	hge->System_Log("Saving file %d", currentSave);

	std::ofstream outputFile;
	std::string outputString;

	//Select the specified save file
	if (currentSave == 0) {
		outputFile.open("Data/Save/file1.sav");
	} else if (currentSave == 1) {
		outputFile.open("Data/Save/file2.sav");
	} else if (currentSave == 2) {
		outputFile.open("Data/Save/file3.sav");
	} else if (currentSave == 3) {
		outputFile.open("Data/Save/file4.sav");
	}

	//Abilities
	for (int i = 0; i < NUM_ABILITIES; i++) outputString += (hasAbility[i] ? "1" : "0");

	//Keys
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			outputString += intToString(numKeys[i][j]);
		}
	}

	//Gems
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			outputString += intToString(numGems[i][j]);
		}
	}

	//Money (3 digits)
	outputString += intToString(money, 3);

	//Upgrades (2 digits)
	for (int i = 0; i < 3; i++) {
		outputString += intToString(numUpgrades[i], 2);
	}

	//Bosses
	for (int i = 0; i < NUM_BOSSES; i++) outputString += (killedBoss[i] ? "0" : "1");

	//Area and position
	outputString += intToString(currentArea);
	outputString += intToString(playerGridX, 3);
	outputString += intToString(playerGridY, 3);

	//Health and mana
	outputString += intToString(smh->player->getHealth() * 4, 3);
	outputString += intToString(smh->player->getMana(), 3);

	//Changed shit
	outputString += changeManager->toString();

	//Stats
	outputString += intToString(numTongueLicks, 5);
	outputString += intToString(numEnemiesKilled, 5);
	outputString += intToString(damageDealt, 5);
	outputString += intToString(damageReceived, 5);

	//Tutorial Man
	outputString += intToString(tutorialManCompleted ? 1 : 0);

	//Exploration data
	unsigned char nextCharToWrite;
	outputString += "\n\n";
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				if (bitManager->addBit(explored[i][j][k])) { //if true, it means the char is full
					nextCharToWrite = bitManager->getCurrentChar();
					outputString += nextCharToWrite;
				}				
			}
		}
	}

	// Now, we write one more char. Even if we didn't fill the char yet,
	// we still don't want to lose any exploration data.
	nextCharToWrite = bitManager->getCurrentChar();
	outputString += nextCharToWrite;

	// Write one more empty char, just to be safe. This is necessary in case we use up
	// exactly the right amount of chars in loading, since it automatically starts
	// looking at the next one.
	nextCharToWrite = 0;
	outputString += nextCharToWrite;

	outputString += "\n";

	//Write the string to the save file
	outputFile.write(outputString.c_str(), outputString.length());

	//Close the file!
	outputFile.close();
	
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

	smh->environment->loadArea(FOUNTAIN_AREA, FOUNTAIN_AREA);
	smh->player->setHealth(playerHealth);
	smh->player->setMana(playerMana);

	playerGridX = smh->player->gridX;
	playerGridY = smh->player->gridY;	
	
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

	infoFile.open("Data/Save/info.dat");
	infoString = "";

	//Build a string with all the file header info
	for (int i = 0; i < 4; i++) {

		//Whether or not the file is empty
		infoString += files[i].empty ? "Y" : "N";
		
		//Time played (6 digits)
		infoString += intToString(files[i].timePlayed, 6);

		//Completion percentage
		infoString += intToString(files[i].completion, 2);

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
			inFile.read(twoBuffer,1);
			twoBuffer[1] = '\0';
			files[i].empty = (strcmp(twoBuffer, "Y") == 0);

			//Time played
			inFile.read(sixBuffer,6);
			files[i].timePlayed = atoi(sixBuffer);

			//Completion
			inFile.read(twoBuffer,2);
			files[i].completion = atoi(twoBuffer);

		}

	}

	inFile.close();

}

/**
 * Returns the current hint number based on what boss has been killed.
 */
int SaveManager::getCurrentHint() {
	if (isBossKilled(FIRE_BOSS2)) {
		return 7;
	} else if (isBossKilled(MUSHROOM_BOSS)) {
		return 6;
	} else if (isBossKilled(DESPAIR_BOSS)) {
		return 5;
	} else if (isBossKilled(DESERT_BOSS)) {
		return 4;
	} else if (isBossKilled(FOREST_BOSS)) {
		return 3;
	} else if (isBossKilled(SNOW_BOSS)) {
		return 2;
	} else if (isBossKilled(FIRE_BOSS)) {
		return 1;
	} else {
		return 0;
	}

}

/**
 * Increments the current save file's time played by the amount of time that has passed
 * since the file was loaded. This is called when the player returns to the main menu or
 * if they manually close the program while still in game state.
 */
void SaveManager::saveTimePlayed() {
	files[currentSave].timePlayed += hge->Timer_GetTime() - timeFileLoaded;
	saveFileInfo();
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

float SaveManager::getDamageModifier() {
	return 1.0 + float(numUpgrades[2]) * 0.05;
}

void SaveManager::killBoss(int boss) {
	killedBoss[boss-240] = true;
}

bool SaveManager::isBossKilled(int boss) {
	return killedBoss[boss-240];
}