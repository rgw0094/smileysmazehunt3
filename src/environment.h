#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

class Tongue;
class hgeParticleSystem;
class CollisionCircle;
class hgeParticleManager;
class hgeRect;
class hgeFont;
class hgeSprite;
class hgeAnimation;
class BaseEnemy;
class SpecialTileManager;
class EvilWallManager;
class TapestryManager;
class SmileletManager;
class Fountain;
class FenwarManager;
class AdviceMan;

struct Timer {
	float duration, startTime;
	float lastClockTickTime;
	float x, y;
	bool playTickSound;
};

struct ParticleStruct {
	hgeParticleSystem *particle;
	float x, y;
	int gridX, gridY;
};

class Environment {

public:
	Environment();
	~Environment();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	void drawGrid(float dt);
	void drawPits(float dt);
	void update(float dt);
	void loadArea(int id, int from, bool playMusic);
	int checkItem(int x, int y);
	int removeItem(int x, int y);
	int collisionAt(float x, float y);
	bool playerCollision(int x, int y,float dt);
	bool enemyCollision(hgeRect *box, BaseEnemy *enemy, float dt);
	void unlockDoor(int gridX, int gridY);
	bool toggleSwitches(hgeRect* box, bool playSoundFarAway, bool playTimerSound);
	bool toggleSwitches(Tongue *tongue);
	bool toggleSwitchAt(int gridX, int gridY, bool playSoundFarAway, bool playTimerSound);
	void toggleSwitch(int id);
	bool hitSigns(Tongue *tongue);
	bool hitSaveShrine(Tongue *tongue);
	bool validPath(int x1, int y1, int x2, int y2, int radius, bool canPass[256]);
	bool validPath(float angle, int x1, int y1, int x2, int y2, int radius, bool canPass[256], bool needsToHitPlayer);
	bool testCollision(hgeRect *box, bool canPass[256], bool ignoreSillyPads);
	bool testCollision(hgeRect *box, bool canPass[256]);
	bool playerOnCylinder(int x, int y);
	void switchCylinders(int id);
	void flipCylinderSwitch(int gridX, int gridY);
	bool isDeepWaterAt(int x, int y);
	bool isShallowWaterAt(int x, int y);
	bool isArrowAt(int x,int y);
	bool isSadSmileletFlowerAt(int x,int y);
	void bombWall(int x,int y);
	void setTerrainCollisionBox(hgeRect *box, int whatFor, int gridX, int gridY);
	void placeSillyPad(int gridX, int gridY);
	bool hasSillyPad(int gridX, int gridY);
	bool destroySillyPad(int gridX, int gridY);
	void reset();
	void updateAdviceMan(float dt);
	bool isAdviceManActive();
	bool isInBounds(int gridX, int gridY);
	float getSwitchDelay();
	void removeParticle(int x,int y);
	void removeAllParticles();
	void addParticle(const char* particle, float x, float y);
	bool shouldEnvironmentDrawCollision(int collision);

	void killSwitchTimer(int gridX, int gridY);
	void updateSwitchTimers(float dt);
	void drawSwitchTimers(float dt);

	//variables
	int areaWidth,areaHeight;		//Width and height of the area in squares
	int ids[256][256];				//ID Layer	
	int variable[256][256];			//Variable Layer
	int terrain[256][256];			//Main Layer
	int collision[256][256];		//Walk Layer
	int item[256][256];				//Item Layer
	int enemyLayer[256][256];		//Enemy Layer
	float activated[256][256];		//What time things were activated on each square
	int screenWidth, screenHeight;	//Number of squares that fit on the screen
	int xGridOffset,yGridOffset;	//Number of squares the top left corner is from (0,0)
	float xOffset,yOffset;			//Number of pixels the player is off alignment with the grid
	int drawX,drawY;				//Location to draw a tile
	int offScreenRange;				//Number of tiles offscreen to draw

	hgeSprite *itemLayer[512];
	SpecialTileManager *specialTileManager;

private:

	EvilWallManager *evilWallManager; //Evil walls which move and try to kill smiley
	TapestryManager *tapestryManager;
	Fountain *fountain;
	SmileletManager *smileletManager;
	AdviceMan *adviceMan;
	hgeRect *collisionBox;
	std::list<Timer> timerList;
	std::list<ParticleStruct> particleList;
	CollisionCircle *collisionCircle;

	hgeAnimation *silverCylinder, *brownCylinder, *blueCylinder, *greenCylinder, *yellowCylinder, *whiteCylinder;
	hgeAnimation *silverCylinderRev, *brownCylinderRev, *blueCylinderRev, *greenCylinderRev, *yellowCylinderRev, *whiteCylinderRev;
	

};

#endif