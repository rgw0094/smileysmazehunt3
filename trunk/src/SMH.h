#ifndef SMH_H_
#define SMH_H_

#define STRICT
#include "smiley.h"
#include <string>
#include <windows.h>
#include <basetsd.h>
#include <dinput.h>
#include "resource.h"
#include <string>
#include <list>

class hgeStringTable;
class HGE;
class hgeResourceManager;
class hgeFont;
class ChangeManager;
class BitManager;
class Player;
class Menu;

//Classes defined here
class SMH;
class SmileyInput;
class GameData;
class SaveManager;

//--------------------------
//------SMH
//--------------------------
class SMH {

public:

	SMH();
	~SMH();

	void init();

	//Utility Functions
	void drawGlobalSprite(const char* sprite, float x, float y);
	void drawSprite(const char* sprite, float x, float y);
	void log(const char* text);

	//Resource access
	hgeFont *getFont(const char* font);

	//Game objects
	GameData *gameData;
	Menu *menu;
	Player *player;
	SaveManager *saveManager;
	SmileyInput *input;

private:

	hgeResourceManager *resources;

};

//----------------------------------------------------------------
//----------------------------------------------------------------
//------------------INPUT-----------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

#define AXIS_MIN -1000			//when joystick is all the way left or up, it returns this
#define AXIS_MAX  1000			//when joystick is all the way right or down, it returns this
#define AXIS_MINCLICK -700		//when axis is below this value, it takes action for that direction
#define AXIS_MAXCLICK 700		//when axis is above this value, it takes action for that direction

//Inputs
#define NUM_INPUTS 10
#define INPUT_LEFT 0
#define INPUT_RIGHT 1
#define INPUT_UP 2
#define INPUT_DOWN 3
#define INPUT_ATTACK 4
#define INPUT_ABILITY 5
#define INPUT_AIM 6
#define INPUT_PREVIOUS_ABILITY 7
#define INPUT_NEXT_ABILITY 8
#define INPUT_PAUSE 9

//Codes for joystick
#define JOYSTICK_LEFT -5
#define JOYSTICK_UP -4
#define JOYSTICK_RIGHT -3
#define JOYSTICK_DOWN -2

//Device types
#define DEVICE_KEYBOARD 1
#define DEVICE_GAMEPAD 2

struct InputStruct {
	bool pressed, prevPressed, editMode;
	int device;
	int code;
};

class SmileyInput {

public:
    
	//Functions
	SmileyInput();
	~SmileyInput();
	void InitInput(HWND hDlg);
	HRESULT InitDirectInput( HWND hDlg );
	VOID FreeDirectInput();
	void UpdateInput();
	bool keyDown(int input);
	bool keyPressed(int input);
	void saveInputs();
	void loadInputs();
	void setEditMode(int whichInput);
	bool isEditModeEnabled(int whichInput);
	const char *getInputName(int whichInput);
	std::string getInputDescription(int whichInput);
	void listenForNewInput(int whichInput);
	float getMouseX();
	float getMouseY();
	bool isMouseInWindow();
	void setMousePosition(float x, float y);

	//Variables
	InputStruct inputs[NUM_INPUTS];
	bool gamePadButtonPressed[128];
	bool useGamePad;
	bool acquiredJoystick;
	bool joystickState[4];

};

//----------------------------------------------------------------
//----------------------------------------------------------------
//------------------DATA------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------

/**
 * Stores info for each enemy id
 */ 
struct EnemyInfo {

	//For all enemies
	int gRow, gCol;
	int enemyType, wanderType, hp, speed, radius, damage, rangedType;
	bool land, shallowWater, deepWater, slime, lava, mushrooms;
	bool immuneToFire, immuneToTongue, immuneToLightning, immuneToStun, immuneToFreeze, invincible;
	int variable1, variable2;
	int numFrames;
	bool hasOneGraphic;
	bool chases, hasRangedAttack;
	int range, delay, projectileSpeed;
	float projectileDamage;
	
};

struct EnemyName {
	int id;
	std::string name;
};

struct Ability {
	char name[32];
	char description[128];
	int manaCost;
	int type;
};

class GameData {

public:

	GameData();
	~GameData();

	EnemyInfo getEnemyInfo(int enemyID);
	Ability getAbilityInfo(int abilityID);
	std::list<EnemyName> getEnemyNames();
	const char *getGameText(const char *text);
	const char *getAreaName(int area);


private:

	void loadEnemyData();
	void loadAbilityData();
	void addEnemyName(int id, std::string name);

	hgeStringTable *enemyStringTable;
	EnemyInfo enemyInfo[99];
	Ability abilities[16];
	std::list<EnemyName> enemyNameList;
	hgeStringTable *gameText;

};


//----------------------------------------------------------------
//----------------------------------------------------------------
//------------------DATA------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------

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
	int numGems[NUM_AREAS][4];
	int numUpgrades[3];
	int money;	

private:
	
	ChangeManager *changeManager;
	BitManager *bitManager;

	SaveFile files[4];
	bool explored[NUM_AREAS][256][256];
	bool killedBoss[12];

};


#endif