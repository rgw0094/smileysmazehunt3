#include <stdlib.h>

#include "SmileyEngine.h"
#include "Player.h"
#include "Environment.h"
#include "boss.h"

extern SMH *smh;

/**
 * Constructor
 */ 
SaveManager::SaveManager() {
	currentSave = -1;
	timeFileSaved = -10.0;
	changeManager = new ChangeManager();
	resetCurrentData();
	loadFileInfo();
}

/**
 * Destructor
 */
SaveManager::~SaveManager() {
	delete changeManager;
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
	for (int i = 0; i < NUM_ABILITIES; i++) hasAbility[i] = false;

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			numKeys[i][j] = 0;
		}
	}

	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			numGems[i][j] = 0;
		}
	}

	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				explored[i][j][k] = false;
			}
		}
	}

	for (int i = 0; i < NUM_AREAS; i++) {
		hasVisitedArea[i] = false;
	}
	hasVisitedArea[FOUNTAIN] = true;

	money = 0;
	tutorialManCompleted = false;
	for (int i = 0; i < 3; i++) numUpgrades[i] = 0;
	currentArea = FOUNTAIN_AREA;
	playerGridX = 0;
	playerGridY = 0;
	playerHealth = 5.0;
	playerMana = INITIAL_MANA;
	currentArea = FOUNTAIN_AREA;

	numTongueLicks = 0;
	numEnemiesKilled = 0;
	pixelsTravelled = 0;

	timeFileLoaded = smh->getRealTime();

}

/**
 * Loads save data from a file into memory.
 */
void SaveManager::load(int fileNumber) {

	smh->hge->System_Log("Loading save file %d", fileNumber);
	changeManager->reset();
	currentSave = fileNumber;

	//Select the specified save file
	BitStream *input = new BitStream();
	if (currentSave == 0) input->open("Data/Save/save1.sav", FILE_READ);
	else if (currentSave == 1) input->open("Data/Save/save2.sav", FILE_READ);
	else if (currentSave == 2) input->open("Data/Save/save3.sav", FILE_READ);
	else if (currentSave == 3) input->open("Data/Save/save4.sav", FILE_READ);
	
	//Load abilties
	for (int i = 0; i < NUM_ABILITIES; i++) hasAbility[i] = input->readBit();

	//Load keys
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			numKeys[i][j] = input->readByte();
		}
	}

	//Load gems
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			numGems[i][j] = input->readByte();
		}
	}

	//Load money
	money = input->readByte();

	//Load upgrades
	for (int i = 0; i < 3; i++) {
		numUpgrades[i] = input->readByte();
	}

	//Load which bosses have been slain
	for (int i = 0; i < NUM_BOSSES; i++) {
		killedBoss[i] = input->readBit();
	}

	//Load player zone and location
	currentArea = input->readByte();
	playerGridX = input->readByte();
	playerGridY = input->readByte();

	//Health and mana
	playerHealth = float(input->readByte()) / 4.0;
	playerMana = float(input->readByte());

	//Load changed shit
	int numChanges = input->readByte();
	for (int i = 0; i < numChanges; i++) {
		changeManager->change(input->readByte(), input->readByte(), input->readByte());
	}

	//Load Stats
	numTongueLicks = input->readByte();
	numEnemiesKilled = input->readByte();
	pixelsTravelled = input->readBits(24);

	//Tutorial Man
	tutorialManCompleted = input->readBit();

	for (int i = 0; i < 3; i++) {
		smh->player->gui->setAbilityInSlot(input->readBits(5), i);
	}

	for (int i = 0; i < NUM_AREAS; i++) {
		hasVisitedArea[i] = input->readBit();
	}

	difficulty = input->readByte();

	//Exploration data
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				explored[i][j][k] = input->readBit();
			}
		}
	}

	input->close();
	delete input;

	timeFileLoaded = smh->getRealTime();

}

/**
 * Saves the current save data in memory to a file.
 */
void SaveManager::save(bool showConfirmation) {

	smh->hge->System_Log("Saving file %d", currentSave);	

	//Heal the player to a minimum of 3 health when they save
	smh->player->setHealth(max(3.0, smh->player->getHealth()));

	//Select the specified save file
	BitStream *output = new BitStream();
	if (currentSave == 0) {
		output->open("Data/Save/save1.sav", FILE_WRITE);
	} else if (currentSave == 1) {
		output->open("Data/Save/save2.sav", FILE_WRITE);
	} else if (currentSave == 2) {
		output->open("Data/Save/save3.sav", FILE_WRITE);
	} else if (currentSave == 3) {
		output->open("Data/Save/save4.sav", FILE_WRITE);
	}

	//Abilities
	for (int i = 0; i < NUM_ABILITIES; i++) output->writeBit(hasAbility[i]);

	//Keys
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4; j++) {
			output->writeByte(numKeys[i][j]);
		}
	}

	//Gems
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 3; j++) {
			output->writeByte(numGems[i][j]);
		}
	}

	//Money
	output->writeByte(money);

	//Upgrades
	for (int i = 0; i < 3; i++) {
		output->writeByte(numUpgrades[i]);
	}

	//Bosses
	for (int i = 0; i < NUM_BOSSES; i++) {
		output->writeBit(killedBoss[i]);
	}

	//Area and position
	output->writeByte(currentArea);
	output->writeByte(playerGridX);
	output->writeByte(playerGridY);

	//Health and mana
	output->writeByte(smh->player->getHealth() * 4);
	output->writeByte(smh->player->getMana());

	//Changed shit
	changeManager->writeToStream(output);

	//Stats
	output->writeByte(numTongueLicks);
	output->writeByte(numEnemiesKilled);
	output->writeBits(pixelsTravelled, 24);

	//Tutorial Man
	output->writeBit(tutorialManCompleted);

	//Selected abilities
	for (int i = 0; i < 3; i++) {
		output->writeBits(smh->player->gui->getAbilityInSlot(i), 5);
	}

	//Which areas have been visited
	for (int i = 0; i < NUM_AREAS; i++) {
		output->writeBit(hasVisitedArea[i] ? 1 : 0);
	}

	//Difficulty
	output->writeByte(difficulty);

	//Exploration data
	for (int i = 0; i < NUM_AREAS; i++) {
		for (int j = 0; j < 256; j++) {
			for (int k = 0; k < 256; k++) {
				output->writeBit(explored[i][j][k]);
			}
		}
	}

	//Close the file!
	output->close();
	delete output;
	
	if (showConfirmation) {
		timeFileSaved = smh->getRealTime();
	}

}

/**
 * Starts a new save file in the specified slot
 */
void SaveManager::startNewGame(int fileNumber) {

	smh->hge->System_Log("Creating new save in file %d", fileNumber);

	files[fileNumber].empty = false;
	files[fileNumber].timePlayed = 0;
	files[fileNumber].completion = 0;
	
	currentSave = fileNumber;

	saveFileInfo();
	resetCurrentData();

	smh->player->setHealth(playerHealth);
	smh->player->setMana(playerMana);
	smh->player->gui->resetAbilities();
}

/**
 * This doesn't actually delete anything. The information in the header file
 * is just reset.
 */
void SaveManager::deleteFile(int file) {

	smh->hge->System_Log("Deleting file %d", file);

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
		infoString += Util::intToString(files[i].timePlayed, 6);

		//Completion percentage
		infoString += Util::intToString(files[i].completion, 2);

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
 * For a couple seconds after saving, text will appear on the screen to confirm to the 
 * player that their game was saved.
 */
void SaveManager::drawSaveConfirmation(float dt) {
	if (smh->getRealTime() < timeFileSaved + 2.5) {
		
		//Determine text alpha - after 1.5 seconds start fading out
		float alpha = 255.0;
		if (smh->getRealTime() > timeFileSaved + 1.55) {
			alpha = (1.0-(smh->getRealTime() - timeFileSaved+1.5)) * 255.0;
		}

		std::string s = "Game File ";
		s += Util::intToString(currentSave + 1);
		s += " Saved!";

		smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(alpha,255.0,255.0,255.0));
		smh->resources->GetFont("inventoryFnt")->printf(512,710,HGETEXT_CENTER, s.c_str());
		smh->resources->GetFont("inventoryFnt")->SetColor(ARGB(255.0,255.0,255.0,255.0));
	}
}

/**
 * Increments the current save file's time played by the amount of time that has passed
 * since the file was loaded. This is called when the player returns to the main menu or
 * if they manually close the program while still in game state.
 */
void SaveManager::saveTimePlayed() {
	files[currentSave].timePlayed += smh->getRealTime() - timeFileLoaded;
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
	return (1.0 + float(numUpgrades[2]) * 0.2) * smh->gameData->getDifficultyModifier(difficulty);
}

float SaveManager::getManaModifier() {
	return 1.0 + float(numUpgrades[1]) * 0.25;
}

void SaveManager::killBoss(int boss) {
	killedBoss[boss-240] = true;
}

bool SaveManager::isBossKilled(int boss) {
	return killedBoss[boss-240];
}