#ifndef SMH_H_
#define SMH_H_

#using <mscorlib.dll>

#define STRICT
#define DIRECTINPUT_VERSION 0x0800

#include "hgeresource.h"
#include "hge.h"
#include <string>
#include <windows.h>
#include <basetsd.h>
#include <fstream>
#include <dinput.h>
#include "resource.h"
#include <list>

class hgeStringTable;
class HGE;
class hgeResourceManager;
class hgeAnimation;
class hgeFont;
class Player;
class Environment;
class MainMenu;
class NPCManager;
class WindowManager;
class EnemyGroupManager;
class EnemyManager;
class LootManager;
class FenwarManager;
class ProjectileManager;
class BossManager;
class ExplosionManager;

//Classes defined here
class AreaChanger;
class GameData;
class SMH;
class SmileyInput;
class SaveManager;
class SoundManager;
class ChangeManager;
class BitStream;
class ScreenEffectsManager;
class DeathEffectManager;
class Console;
class PopupMessageManager;

//Constants
#define PI 3.14159265357989232684

//NPCS
#define MONOCLE_MAN_NPC_ID 13
#define MONOCLE_MAN_TEXT_ID 902
#define SPIERDYKE_TEXT_ID 5
#define BILL_CLINTON_TEXT_ID 8
#define BILL_CLINTON_TEXT_ID2 19

//Difficulty
#define VERY_EASY 0
#define EASY 1
#define MEDIUM 2
#define HARD 3
#define VERY_HARD 4

//Gameplay values
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define NUM_NPCS 99
#define PLAYER_WIDTH 61
#define PLAYER_HEIGHT 72

//Resource Groups
#define RES_MENU 10
#define RES_CREDITS 11
#define RES_CINEMATIC 12
#define RES_WORLDMAP 13
#define RES_PHYREBOZZ 100
#define RES_PORTLYPENGUIN 101
#define RES_GARMBORN 102
#define RES_CORNWALLIS 103
#define RES_MUSHBOOM 104
#define RES_CALYPSO 105
#define RES_BARTLI 106
#define RES_KINGTUT 107
#define RES_LOVECRAFT 108
#define RES_BARVINOID 109
#define RES_FENWAR 110

#define NUM_BOSSES 12

//Special editor IDs
#define DRAW_AFTER_SMILEY 990

//Abilities
#define NUM_ABILITIES 12
#define NO_ABILITY 12
#define CANE 0
#define FIRE_BREATH 1
#define FRISBEE 2
#define SPRINT_BOOTS 3
#define LIGHTNING_ORB 4
#define REFLECTION_SHIELD 5
#define SILLY_PAD 6
#define WATER_BOOTS 7
#define ICE_BREATH 8
#define SHRINK 9
#define TUTS_MASK 10
#define HOVER 11

//Ability types
#define PASSIVE 0
#define ACTIVATED 1
#define HOLD 2

//Level ids
#define NUM_AREAS 11
#define FOUNTAIN_AREA 0
#define OLDE_TOWNE 1
#define TUTS_TOMB 2
#define FOREST_OF_FUNGORIA 3
#define SESSARIA_SNOWPLAINS 4
#define WORLD_OF_DESPAIR 5
#define SERPENTINE_PATH 6
#define CASTLE_OF_EVIL 7
#define SMOLDER_HOLLOW 8
#define CONSERVATORY 9
#define DEBUG_AREA 10

//State
#define MENU 10
#define GAME 20

//Directions
#define NUM_DIRECTIONS 8
#define DOWN 0
#define LEFT 1
#define RIGHT 2
#define UP 3
#define UP_LEFT 4
#define UP_RIGHT 5
#define DOWN_LEFT 6
#define DOWN_RIGHT 7

//Item Layer
#define NUM_ITEMS 9
#define NONE 0
#define RED_KEY 1
#define YELLOW_KEY 2
#define GREEN_KEY 3
#define BLUE_KEY 4
#define SMALL_GEM 5
#define MEDIUM_GEM 6
#define LARGE_GEM 7
#define HEALTH_ITEM 9
#define MANA_ITEM 8

//Colors
#define RED 0
#define YELLOW 1
#define GREEN 2
#define BLUE 3
#define BLACK 4

//Collision Layer
#define NUM_COLLISION 36
#define WALKABLE 0
#define UNWALKABLE 1
#define RED_KEYHOLE 2
#define YELLOW_KEYHOLE 3
#define GREEN_KEYHOLE 4
#define BLUE_KEYHOLE 5
#define EVIL_DOOR 6
#define SHALLOW_WATER 7
#define DEEP_WATER 8
#define UP_ARROW 9
#define RIGHT_ARROW 10
#define DOWN_ARROW 11
#define LEFT_ARROW 12
#define SLIME 13
#define SPRING_PAD 14
#define FIRE_DESTROY 15
#define WALK_LAVA 16
#define NO_WALK_LAVA 17
#define UNWALKABLE_PROJECTILE 18
#define RED_WARP 19
#define BLUE_WARP 20
#define YELLOW_WARP 21
#define GREEN_WARP 22
#define SPIN_ARROW_SWITCH 23
#define PIT 24
#define FOUNTAIN 25
#define SAVE_SHRINE 26
#define SIGN 27
#define ICE 28
#define MIRROR_UP_LEFT 29
#define MIRROR_UP_RIGHT 30
#define MIRROR_DOWN_RIGHT 31
#define MIRROR_DOWN_LEFT 32
#define MIRROR_SWITCH 33
#define ENEMY_NO_WALK 34
#define GREEN_WATER 35
#define DIZZY_MUSHROOM_1 36
#define DIZZY_MUSHROOM_2 37
#define BOMB_PAD_UP 38
#define BOMB_PAD_DOWN 39
#define BOMBABLE_WALL 40
#define HOVER_PAD 41
#define WHITE_CYLINDER_DOWN 42
#define YELLOW_CYLINDER_DOWN 43
#define GREEN_CYLINDER_DOWN 44
#define BLUE_CYLINDER_DOWN 45
#define BROWN_CYLINDER_DOWN 46
#define SILVER_CYLINDER_DOWN 47
#define SHRINK_TUNNEL_SWITCH 48
#define SHRINK_TUNNEL_HORIZONTAL 49
#define SHRINK_TUNNEL_VERTICAL 50
#define SHALLOW_GREEN_WATER 51
#define EVIL_WALL_POSITION 52
#define EVIL_WALL_TRIGGER 53
#define EVIL_WALL_DEACTIVATOR 54
#define EVIL_WALL_RESTART 55
#define FLAME 56
#define SUPER_SPRING 57
#define WHITE_CYLINDER_UP 58
#define YELLOW_CYLINDER_UP 59
#define GREEN_CYLINDER_UP 60
#define BLUE_CYLINDER_UP 61
#define BROWN_CYLINDER_UP 62
#define SILVER_CYLINDER_UP 63
#define SMILELET 64
#define SMILELET_FLOWER_SAD 65
#define SMILELET_FLOWER_HAPPY 66
#define NO_WALK_PIT 67
#define FAKE_PIT 68
#define NO_WALK_WATER 69
#define FAKE_COLLISION 70
//...
#define WHITE_SWITCH_LEFT 74
#define YELLOW_SWITCH_LEFT 75
#define GREEN_SWITCH_LEFT 76
#define BLUE_SWITCH_LEFT 77
#define BROWN_SWITCH_LEFT 78
#define SILVER_SWITCH_LEFT 79
//...
#define WHITE_SWITCH_RIGHT 90
#define YELLOW_SWITCH_RIGHT 91
#define GREEN_SWITCH_RIGHT 92
#define BLUE_SWITCH_RIGHT 93
#define BROWN_SWITCH_RIGHT 94
#define SILVER_SWITCH_RIGHT 95
//...
#define PLAYER_START 224
#define PLAYER_END 225

struct Point {
	int x, y;
};

//----------------------------------------------------------------
//------------------SMH-------------------------------------------
//----------------------------------------------------------------
// This is the global game manager class. When an object wants to
// access another object in the Smiley world it can do it through
// this class. This also manages game state and provides useful
// utility methods. The update and draw methods are called directly
// by HGE, and are therefore the highest level of updating/drawing
// logic.
//----------------------------------------------------------------
class SMH {

public:

	SMH(HGE *hge);
	~SMH();

	//Public methods
	bool updateGame();
	void drawGame();
	void init();
	bool isDebugOn();
	void toggleDebugMode();
	void setGameTime(float dt);
	void enterGameState(int newState);
	int getCurrentFrame();
	int getGameState();
	float getGameTime();
	float getRealTime();
	void setScreenColor(int color, float alpha);
	float getScreenColorAlpha();

	//Utility Functions
	void drawCollisionBox(hgeRect *box, int color);
	void drawGlobalSprite(const char* sprite, float x, float y);
	void drawSprite(const char* sprite, float x, float y);
	void drawSprite(const char* sprite, float x, float y, float width, float height);
	int getScreenX(int x);
	int getScreenY(int y);
	void log(const char* text);
	int randomInt(int min, int max);
	float randomFloat(float min, float max);
	void drawScreenColor(int color, float alpha);
	float timePassedSince(float time);
	float getFlashingAlpha(float n);
	void setDebugText(std::string);
	
	//Game objects
	Console *console;
	AreaChanger *areaChanger;
	BossManager *bossManager;
	EnemyGroupManager *enemyGroupManager;
	EnemyManager *enemyManager;
	Environment *environment;
	FenwarManager *fenwarManager;
	GameData *gameData;
	HGE *hge;
	SmileyInput *input;
	LootManager *lootManager;
	MainMenu *menu;
	NPCManager *npcManager;
	Player *player;
	ProjectileManager *projectileManager;
	hgeResourceManager *resources;
	SaveManager *saveManager;
	SoundManager *soundManager;
	WindowManager *windowManager;
	ScreenEffectsManager *screenEffectsManager;
	ExplosionManager *explosionManager;
	DeathEffectManager *deathEffectManager;
	PopupMessageManager *popupMessageManager;

private:

	void doDebugInput(float dt);

	float gameTime;
	float timeInState;
	int gameState;
	int frameCounter;
	bool debugMode;
	int screenColor;
	float screenColorAlpha;
	std::string debugText;

};

//----------------------------------------------------------------
//------------------INPUT-----------------------------------------
//----------------------------------------------------------------
// Manages player input and encapsulates shitty DirectInput code.
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
//------------------DATA------------------------------------------
//----------------------------------------------------------------
// Provides a centralized location for retrieving all game data.
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
	int variable1, variable2, variable3;
	int numFrames;
	bool hasOneGraphic;
	bool chases, hasRangedAttack;
	int range, delay, projectileSpeed;
	float projectileDamage;
	float projectileHoming;
	
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
	int getNumTotalGemsInArea(int area, int gemType);
	const char *getGameText(const char *text);
	const char *getAreaName(int area);
	float getDifficultyModifier(int difficulty);
	int getNumEnemies();

private:

	void loadEnemyData();
	void loadAbilityData();
	void addEnemyName(int id, std::string name);
	void initializeGemCounts();

	hgeStringTable *enemyStringTable;
	EnemyInfo enemyInfo[99];
	Ability abilities[16];
	std::list<EnemyName> enemyNameList;
	hgeStringTable *gameText;
	int totalGemCounts[NUM_AREAS][3];

};


//----------------------------------------------------------------
//------------------SAVE MANAGER----------------------------------
//----------------------------------------------------------------
// Manages saving and loading games. When a file is loaded, all of
// its information is stored internally which is then retrieved or
// changed during gameplay.
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
	float getManaModifier();
	void killBoss(int boss);
	bool isBossKilled(int boss);
	void explore(int gridX, int gridY);
	bool isExplored(int gridX, int gridY);
	int getCurrentHint();

	//Stats
	int numTongueLicks;
	int numEnemiesKilled;
	int pixelsTravelled;

	//Save data
	float timeFileLoaded;	//used to track playing time
	int currentSave;
	int currentArea;
	bool adviceManEncounterCompleted;
	int playerGridX, playerGridY;
	float playerHealth, playerMana;
	int difficulty;

	//Stuff in inventory
	bool hasAbility[12];
	int numKeys[5][4];
	int numGems[NUM_AREAS][3];
	int numUpgrades[3];
	bool hasVisitedArea[NUM_AREAS];
	int money;	

private:
	
	ChangeManager *changeManager;

	SaveFile files[4];
	bool explored[NUM_AREAS][256][256];
	bool killedBoss[12];

};


//----------------------------------------------------------------
//------------------BIT STREAM------------------------------------
//----------------------------------------------------------------
// This class allows you to easily read/write bits and bytes to
// a file to make up for the mind boggling shortcomings of C++.
//----------------------------------------------------------------
#define FILE_READ 0
#define FILE_WRITE 1

class BitStream {

public:
	BitStream();
	~BitStream();

	void open(std::string fileName, int mode);
	void writeByte(int byte);
	bool writeBit(bool bit);
	void writeBits(int data, int numBits);
	int readByte();
	bool readBit();
	int readBits(int numBits);
	void close();
	int getNumBitsRead();
	int getNumBitsWritten();
	static void test();

private:
	bool isOpen;
	int mode, numRead, numWritten;
	std::ifstream inFile;
	std::ofstream outFile;
	std::string outString;
	unsigned char byte;
	int counter;
};

//----------------------------------------------------------------
//------------------SOUND MANAGER---------------------------------
//----------------------------------------------------------------
// Encapsulates all sound logic. This class should be used to 
// play any sounds or music.
//----------------------------------------------------------------
struct Sound {
	std::string name;
	float lastTimePlayed;
};

class SoundManager {

public:

	SoundManager();
	~SoundManager();

	//Methods
	void playMusic(char *musicName);
	void playAreaMusic(int area);
	void stopMusic();
	void fadeOutMusic();
	void playPreviousMusic();
	void setMusicVolume(int newVolume);
	void setSoundVolume(int soundVolume);
	void playEnvironmentEffect(char *effect, bool loop);
	void stopEnvironmentChannel();
	void playAbilityEffect(char *effect, bool loop);
	void stopAbilityChannel();
	void playSound(const char* sound);
	void playSound(const char* sound, float delay);
	void playSwitchSound(int gridX, int gridY, bool alwaysPlaySound);
	int getMusicVolume();
	int getSoundVolume();

private:

	HCHANNEL musicChannel;
	HCHANNEL abilityChannel;		//Audio channel for player ability sound effects
	HCHANNEL environmentChannel;	//Audio channel for environment sound effects
	
	std::list<Sound> lastPlayTimes;
	std::string currentMusic;
	std::string previousMusic;
	int previousMusicPosition;
	int musicVolume;
	int soundVolume;
	float lastSwitchSoundTime;

};

//----------------------------------------------------------------
//------------------AREA CHANGER----------------------------------
//----------------------------------------------------------------
// This class is used to relocate Smiley to either a new area
// or a new location within the same area. It will relocate Smiley
// and display a loading effect.
//----------------------------------------------------------------
class AreaChanger {

public:

	AreaChanger();
	~AreaChanger();

	//methods
	void draw(float dt);
	void update(float dt);
	void changeArea(int destinationX, int destinationY, int destinationArea);
	bool isChangingArea();
	void displayNewAreaName();

private:

	int state;
	int destinationX;
	int destinationY;
	int destinationArea;

	bool doneZoomingIn;
	float timeLevelLoaded;
	float zoneTextAlpha;
	float loadingEffectScale;

};

//----------------------------------------------------------------
//------------------ CHANGE MANAGER ------------------------------
//----------------------------------------------------------------
// Used to manage all changes to the game world. Internally, it is
// a linked list of objects representing a single square in one area
// that has been changed from its original state. This only supports
// boolean states at each square.
//----------------------------------------------------------------
struct Change {
	int x,int y;
	int area;
};

class ChangeManager {

public:

	ChangeManager();
	~ChangeManager();

	void change(int area, int x, int y);
	bool isChanged(int area, int x, int y);
	void reset();
	void writeToStream(BitStream *stream);

private:
	std::list<Change> theChanges;
	void addChange(int area, int x, int y);
	bool removeChange(int area, int x, int y);
};

//----------------------------------------------------------------
//-------------- SCREEN EFFECTS MANAGER --------------------------
//----------------------------------------------------------------
// Manages and applies screen effects such as shaking.
//----------------------------------------------------------------
class ScreenEffectsManager {

public:

	ScreenEffectsManager();
	~ScreenEffectsManager();

	void update(float dt);
	void applyEffect();
	bool isEffectActive();
	void stopEffect();

	void startShaking(float duration, float amount);

private:

	void startEffect(int effect, float duration, float amount);

	int currentEffect;
	float effectDuration;
	float effectValue;
	float timeEffectStarted;

	float xOffset, yOffset, rotation, hScale, vScale;

};

//----------------------------------------------------------------
//-------------- DEATH EFFECT MANAGER ----------------------------
//----------------------------------------------------------------
// Draws the death effect and then returns the player to the main menu
//----------------------------------------------------------------
struct Letter {
	float yOffset;
	bool startedYet;
	hgeSprite *sprite;
};

class DeathEffectManager {

public:

	DeathEffectManager();
	~DeathEffectManager();

	//Draw methods
	void draw(float dt);
	void update(float dt);
	void beginEffect();
	bool isActive();

private:

	void initLetters();
	void enterState(int newState);

	float alpha;
	float smileyScale;
	int state;
	float timeEnteredState;
	bool active;
	float textAlpha;
	Letter letters[8];

};

//----------------------------------------------------------------
//---------------------------- CONSOLE ----------------------------
//----------------------------------------------------------------
// Toggle by pressing ~ to allow debug input
//----------------------------------------------------------------
class Console {

public:

	Console();
	~Console();

	void update(float dt);
	void draw(float dt);
	void toggle();

private:

	void write (std::string text, int toggled);

	bool active;
	bool debugMovePressed;
	float lastDebugMoveTime;
	int lineNum;

};

//----------------------------------------------------------------
//---------------------- POPUPMESSAGEMANAGER----------------------
//----------------------------------------------------------------
// Shows messages on the screen
//----------------------------------------------------------------
class PopupMessageManager {

public:

	PopupMessageManager();
	~PopupMessageManager();

	void update(float dt);
	void draw(float dt);

	void showSaveConfirmation();
	void showNewAdvice(int advice);
	void showFullMana();
	void showFullHealth();

private:
	
	void startMessage(float duration);

	std::string message;
	float messageDuration;
	float timeMessageStarted;
	float messageAlpha;
	bool adviceManMessageActive;
	int advice;

};

//----------------------------------------------------------------
//------------------ UTIL ----------------------------------------
//----------------------------------------------------------------
// This class is for static utility functions outside the scope of the
// Smiley world, such as string conversions and shit.
//----------------------------------------------------------------
class Util {

public:

	static int roundUp(float num) {
		if (num > (int)num) return (int)num + 1;
		else return (int)num;
	}

	/**
	 * Returns the distance between 2 points
	 */
	static int distance(int x1, int y1, int x2, int y2) {
		if (x1 == x2) return abs(y1 - y2);
		if (y1 == y2) return abs(x1 - x2);
		return sqrt(float((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
	}

	/**
	 * Returns the specified integer as a string because the designers of C were too 
	 * distracted by their beards to write a language that doESNT SUCK ASS FUCK SHIT
	 */
	static std::string intToString(int n) {
		std::string numberString = "";
		char number[10];
		itoa(n, number, 10);
		numberString += number;
		return numberString;
	}

	/**
	 * Returns the int as a string, with the given number of digits
	 */
	static std::string intToString(int number, int digits) {
		std::string returnString;
		returnString = intToString(number);
		while (returnString.size() < digits) {
			returnString.insert(0,"0");			
		}
		return returnString;
	}

	/**
	 * Returns whether or not id is the id of an on cylinder switch
	 */
	static bool isCylinderSwitchLeft(int id) {
		return (id == WHITE_SWITCH_LEFT || id == YELLOW_SWITCH_LEFT || id == GREEN_SWITCH_LEFT ||
			id == BLUE_SWITCH_LEFT || id == BROWN_SWITCH_LEFT || id == SILVER_SWITCH_LEFT);
	}

	/**
	 * Returns whether or not id is the id of an off cylinder switch
	 */
	static bool isCylinderSwitchRight(int id) {
		return (id == WHITE_SWITCH_RIGHT || id == YELLOW_SWITCH_RIGHT || id == GREEN_SWITCH_RIGHT ||
				id == BLUE_SWITCH_RIGHT || id == BROWN_SWITCH_RIGHT || id == SILVER_SWITCH_RIGHT);
	}

	/**
	 * Returns whether or not id is the id of a down cylinder
	 */
	static bool isCylinderDown(int id) {
		return (id == WHITE_CYLINDER_DOWN || id == YELLOW_CYLINDER_DOWN || id == GREEN_CYLINDER_DOWN ||
				id == BLUE_CYLINDER_DOWN || id == BROWN_CYLINDER_DOWN || id == SILVER_CYLINDER_DOWN);
	}

	/**
	 * Returns whether or not id is the id of an up cylinder
	 */
	static bool isCylinderUp(int id) {
		return (id == WHITE_CYLINDER_UP || id == YELLOW_CYLINDER_UP || id == GREEN_CYLINDER_UP ||
				id == BLUE_CYLINDER_UP|| id == BROWN_CYLINDER_UP || id == SILVER_CYLINDER_UP);
	}

	/**
	 * Returns whether or not id is the id of an arrow pad
     */ 
	static bool isArrowPad(int id) {
		return (id == UP_ARROW || id == RIGHT_ARROW || id == DOWN_ARROW || id == LEFT_ARROW);
	}

	/**
	 * Returns whether or not id is one that should use the gay fix
	 */
	static bool isTileForGayFix(int id) {
		return (isArrowPad(id) || id == SPRING_PAD || id == ICE || id == SUPER_SPRING || id == PIT);
	}

	/**
	 * Returns the grid x coordinate that x appears in
	 */
	static int getGridX(int x) {
		return (x - x%64) / 64;
	} 

	/**
	 * Returns the grid y coordinate that y appears in
	 */
	static int getGridY(int y) {
		return (y - y%64) / 64;
	}

	/**
	 * Returns the angle between (x1,y1) and (x2,y2)
	 */
	static float getAngleBetween(int x1, int y1, int x2, int y2) {

		float angle;

		if (x1 == x2) {
			if (y1 > y2) {
				angle = 3.0*PI/2.0;
			} else {
				angle=PI/2.0;
			}
		} else {
			angle = atan(float(y2-y1)/float(x2-x1));
			if (x1 - x2 > 0) angle += PI;
		}

		return angle;

	}

	/**
	 * Normalizes an angle to be between 0 and 2 pi
	 */
	static float normalizeAngle(float angle) {
		while (angle < 0) angle += 2*PI;
		while (angle > 2*PI) angle -= 2*PI;
		return angle;
	}

	/**
	 * Returns whether or not a normalized angle is between two normalized angles
	 */
	static bool isAngleBetween(float testAngle, float minAngle, float maxAngle) {
		if (minAngle < maxAngle) { //easy case -- the angles do not span over 0
			return (testAngle >= minAngle && testAngle <= maxAngle);	
		} else if (minAngle == maxAngle) { //the only way it can be between is if it's equal
			return (testAngle == maxAngle);
		} else if (minAngle > maxAngle) { //in this case, 0 is between the min and the max. So if test angle is < max or > min then it's between
			return (testAngle > minAngle || testAngle < maxAngle);
		}

		return false;
	}

	/**
	 * Returns whether or not a collision layer id is a warp.
	 */
	static bool isWarp(int id) {
		return (id == BLUE_WARP || id == RED_WARP || id == GREEN_WARP || id == YELLOW_WARP);
	}

	/**
	 * Returns the parent area of the given area. Only the 5 parent areas have keys, so this method
	 * is used to determine which of these 5 areas to save the key to!! The number returned is the
	 * [area] index of SaveManager.numKeys[area][key color] for the parent area.
	 */
	static int getKeyIndex(int area) {
		switch (area) {
			case OLDE_TOWNE:
			case TUTS_TOMB:
			case SMOLDER_HOLLOW:
				return 0;
				break;
			case FOREST_OF_FUNGORIA:
				return 1;
				break;
			case SESSARIA_SNOWPLAINS:
				return 2;
				break;
			case WORLD_OF_DESPAIR:
				return 3;
				break;
			case CASTLE_OF_EVIL:
				return 4;
				break;
		}
		return -1;
	}

};

#endif