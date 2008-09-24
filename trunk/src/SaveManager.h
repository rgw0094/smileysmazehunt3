#ifndef _SAVEMANAGER_H_
#define _SAVEMANAGER_H_

class ChangeManager;
class BitManager;

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
	void saveTimePlayed();
	void startNewGame(int fileNumber);
	void saveFileInfo();
	void loadFileInfo();
	bool isFileEmpty(int file);
	int getTimePlayed(int file);
	int getCompletion(int file);
	void incrementTimePlayed(int file, int amount);
	void change(int gridX, int gridY);
	bool isTileChanged(int gridX, int gridY);
	float getDamageModifier();
	void killBoss(int boss);
	bool isBossKilled(int boss);
	void explore(int gridX, int gridY);
	bool isExplored(int gridX, int gridY);

	//Stats
	int numTongueLicks;
	int numEnemiesKilled;
	float damageDealt;
	float damageReceived;

	//Save data
	float timeFileLoaded;	//used to track playing time
	int currentSave;
	int currentArea;
	bool tutorialManCompleted;
	int playerGridX, playerGridY;
	float playerHealth, playerMana;

	//Stuff in inventory
	bool hasAbility[12];
	int numKeys[5][4];
	int numGems[9][4];
	int numUpgrades[3];
	int money;	

private:
	
	ChangeManager *changeManager;
	BitManager *bitManager;

	SaveFile files[4];
	bool explored[9][256][256];
	bool killedBoss[12];

};

#endif