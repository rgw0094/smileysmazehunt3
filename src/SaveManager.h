#ifndef _SAVEMANAGER_H_
#define _SAVEMANAGER_H_

class ChangeManager;

struct SaveFile {
	bool empty;
	int timePlayed;
	int completion;
};

class SaveManager {

public:

	SaveManager();
	~SaveManager();
	
	//Methods
	void resetCurrentData();
	void load(int fileNumber);
	void save();
	void deleteFile(int fileNumber);
	void startNewGame(int fileNumber);

	void saveFileInfo();
	void loadFileInfo();
	bool isFileEmpty(int file);
	int getTimePlayed(int file);
	int getCompletion(int file);
	void incrementTimePlayed(int file, int amount);

	void explore(int gridX, int gridY);
	bool isExplored(int gridX, int gridY);

	//Save data
	int currentSave;
	int currentArea;					//The main area Smiley is currently in
	bool killedBoss[12];
	int playerGridX, playerGridY;
	int currentHint;

	//Stuff in inventory
	bool hasAbility[12];
	int numKeys[5][4];
	int numGems[9][4];
	int numUpgrades[3];
	int money;

	ChangeManager *changeManager;

private:

	SaveFile files[4];
	bool explored[9][32][32];

};

#endif