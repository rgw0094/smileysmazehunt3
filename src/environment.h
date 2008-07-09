#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

#include "smiley.h"
#include "collisioncircle.h"
#include "enemy.h"
#include "hge include/hgeparticle.h"
#include "BaseEnemy.h"
#include "Tongue.h"

class Environment {

public:
	Environment();
	~Environment();

	//methods
	void draw(float dt);
	void drawAfterSmiley(float dt);
	void drawGrid(float dt);
	void update(float dt);
	void toggleGrid();
	void loadArea(int id, int from, int playerX, int playerY);
	int gatherItem(int x, int y);
	int collisionAt(float x, float y);
	bool playerCollision(int x, int y,float dt);
	bool enemyCollision(hgeRect *box, BaseEnemy *enemy, float dt);
	void unlockDoor(int gridX, int gridY);
	bool toggleSwitches(hgeRect* box);
	bool toggleSwitches(Tongue *tongue);
	bool toggleSwitchAt(int gridX, int gridY);
	void hitSillyPads(Tongue *tongue);
	void hitSigns(Tongue *tongue);
	void hitSaveShrine(Tongue *tongue);
	bool validPath(int x1, int y1, int x2, int y2, int radius, bool canPass[256]);
	void drawFountain();
	bool testCollision(hgeRect *box, bool canPass[256]);
	bool playerOnCylinder(int x, int y);
	void switchCylinders(int id);
	void flipCylinderSwitch(int gridX, int gridY);
	bool isDeepWaterAt(int x, int y);
	bool isShallowWaterAt(int x, int y);
	void bombWall(int x,int y);

	//Objects
	hgeRect *collisionBox;
	hgeFont *zoneFont;

	//variables
	int areaWidth,areaHeight;		//Width and height of the area in squares
	int ids[256][256];				//ID Layer	
	int variable[256][256];			//Variable Layer
	int terrain[256][256];			//Main Layer
	int collision[256][256];		//Walk Layer
	int item[256][256];				//Item Layer
	int enemyLayer[256][256];		//Enemy Layer
	int save[256][256];				//Save Layer
	float activated[256][256];		//What time things were activated on each square
	bool hasSillyPad[256][256];
	float timeSillyPadPlaced[256][256];
	bool changed[256][256];			//If the state of this square has changed since the area loaded
	int squareSize;					//Size to make the squares
	int screenWidth, screenHeight;	//Number of squares that fit on the screen
	bool showGrid;					//Whether or not to show grid lines
	int xGridOffset,yGridOffset;	//Number of squares the top left corner is from (0,0)
	float xOffset,yOffset;			//Number of pixels the player is off alignment with the grid
	int startX, int startY;			//Level entrance
	bool fountainOnScreen;
	float timeLevelLoaded;
	int zoneTextAlpha;
	float drawX,drawY;				//Location to draw a tile
	int offScreenRange;				//Number of tiles offscreen to draw
	std::string zoneName;

	//Animations
	hgeAnimation *silverCylinder, *brownCylinder, *blueCylinder, *greenCylinder, *yellowCylinder, *whiteCylinder;
	hgeAnimation *silverCylinderRev, *brownCylinderRev, *blueCylinderRev, *greenCylinderRev, *yellowCylinderRev, *whiteCylinderRev;
	
	//Particles
	hgeParticleManager *environmentParticles;

	//IO Shit
	std::ifstream areaFile;				//Input stream to read area data
	char threeBuffer[3];

};

#endif