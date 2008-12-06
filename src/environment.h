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
class TutorialMan;

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
	void loadArea(int id, int from);
	int gatherItem(int x, int y);
	int collisionAt(float x, float y);
	bool playerCollision(int x, int y,float dt);
	bool enemyCollision(hgeRect *box, BaseEnemy *enemy, float dt);
	void unlockDoor(int gridX, int gridY);
	bool toggleSwitches(hgeRect* box);
	bool toggleSwitches(hgeRect* box, bool playSoundFarAway);
	bool toggleSwitches(Tongue *tongue);
	bool toggleSwitchAt(int gridX, int gridY, bool playSoundFarAway);
	void toggleSwitch(int id);
	bool hitSigns(Tongue *tongue);
	bool hitSaveShrine(Tongue *tongue);
	bool validPath(int x1, int y1, int x2, int y2, int radius, bool canPass[256]);
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
	void addTimedTile(int gridX, int gridY, int tile, float duration);
	bool isTimedTileAt(int gridX, int gridY);
	bool isTimedTileAt(int gridX, int gridY, int tile);
	void reset();
	void updateTutorialMan(float dt);
	bool isTutorialManActive();
	bool Environment::isInBounds(int gridX, int gridY);
    
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

	hgeParticleManager *environmentParticles;
	hgeSprite *itemLayer[512];

private:

	SpecialTileManager *specialTileManager;
	EvilWallManager *evilWallManager; //Evil walls which move and try to kill smiley
	TapestryManager *tapestryManager;
	Fountain *fountain;
	SmileletManager *smileletManager;
	TutorialMan *tutorialMan;
	hgeRect *collisionBox;

	hgeAnimation *silverCylinder, *brownCylinder, *blueCylinder, *greenCylinder, *yellowCylinder, *whiteCylinder;
	hgeAnimation *silverCylinderRev, *brownCylinderRev, *blueCylinderRev, *greenCylinderRev, *yellowCylinderRev, *whiteCylinderRev;
	

};

#endif