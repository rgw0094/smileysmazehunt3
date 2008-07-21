/**
 * Smiley's Maze Hunt 3 global functions
 */
#include "smiley.h"
#include "environment.h"
#include "player.h"
#include "SaveManager.h"
#include "EnemyManager.h"
#include "EnemyGroupManager.h"
#include "LootManager.h"
#include "Projectiles.h"
#include "NPCManager.h"
#include "boss.h"
#include "WindowManager.h"

extern bool debugMode;
extern float timePlayed, gameStart;
extern int gameState;
extern hgeResourceManager *resources;
extern float gameTime;
extern int frameCounter;

extern HGE *hge;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern SaveManager *saveManager;
extern EnemyGroupManager *enemyGroupManager;
extern ProjectileManager *projectileManager;
extern LootManager *lootManager;
extern NPCManager *npcManager;
extern EnemyManager *enemyManager;
extern BossManager *bossManager;
extern WindowManager *windowManager;

/**
 * Switches the game state
 */
void enterGameState(int newState) {

	//If leaving the menu, clear all the menu resources
	if (gameState == MENU) {	
		resources->Purge(RES_MENU);
	}

	gameState = newState;

	if (gameState == GAME) {
		gameTime = 0;
		frameCounter = 0;
	}
}

/**
 * Draw a collision box. The color options are RED, GREEN, or BLUE. If any other value is
 * is specified it will default to black;
 */
void drawCollisionBox(hgeRect *box, int color) {

	int r = (color == RED) ? 255 : 0;
	int g = (color == GREEN) ? 255 : 0;
	int b = (color == BLUE) ? 255 : 0;

	int x1 = box->x1 - theEnvironment->xGridOffset*theEnvironment->squareSize - theEnvironment->xOffset;
	int x2 = box->x2 - theEnvironment->xGridOffset*theEnvironment->squareSize - theEnvironment->xOffset;
	int y1 = box->y1 - theEnvironment->yGridOffset*theEnvironment->squareSize - theEnvironment->yOffset;
	int y2 = box->y2 - theEnvironment->yGridOffset*theEnvironment->squareSize - theEnvironment->yOffset;

	hge->Gfx_RenderLine(x1, y1, x2, y1, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x2, y1, x2, y2, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x2, y2, x1, y2, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x1, y2, x1, y1, ARGB(255,r,g,b));

}


/**
 * Set a collision box for the speicifed collision type decalared in smiley.
 * This allows different things to have different shaped collision boxes.
 */
void setTerrainCollisionBox(hgeRect *box, int whatFor, int gridX, int gridY) {
	if (whatFor == FOUNTAIN) {
		//Fountain
		box->Set((gridX-1)*64,gridY*64 + 35,(gridX+2)*64,(gridY+1)*64 + 10);
	} else {
		box->SetRadius(gridX*64+32,gridY*64+31,31);
	}
}


/**
 * Return whether a grid coordinate is in bounds or not
 */
bool inBounds(int gridX, int gridY) {
	return (gridX >= 0 && gridY >= 0 && gridX < theEnvironment->areaWidth && gridY < theEnvironment->areaHeight);
}


/**
 * Returns the screen x position given the global x position
 */
int getScreenX(int x) {
	return x - theEnvironment->xGridOffset*theEnvironment->squareSize - theEnvironment->xOffset;
}


/**
 * Returns the screen y position given the global y position
 */
int getScreenY(int y) {
	return y - theEnvironment->yGridOffset*theEnvironment->squareSize - theEnvironment->yOffset;										  
}


/**
 * Returns the distance between 2 points
 */
int distance(int x1, int y1, int x2, int y2) {
	if (x1 == x2) return abs(y1 - y2);
	if (y1 == y2) return abs(x1 - x2);
	return sqrt(float((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1)));
}

/**
 * Sets time to a string in the format HH:MM:SS for the specified number of seconds
 */ 
char *getTime(int time) {

	char * timeString = (char*)malloc(100);
	char hours[2];
	char minutes[2];
	char seconds[2];
	char temp[2];

	//Get number of hours, minutes, seconds
	itoa((time - (time % 3600)) / 3600, hours, 10);
	time -= 3600*atoi(hours);
	itoa((time - (time % 60)) / 60, minutes, 10);
	time -= 60*atoi(minutes);
	itoa(time, seconds, 10);

	if (strlen(minutes) == 1) {
		strcpy(temp, minutes);
		strcpy(minutes,"0");
		strcat(minutes, temp);
	}

	if (strlen(seconds) == 1) {
		strcpy(temp, seconds);
		strcpy(seconds,"0");
		strcat(seconds, temp);
	}

	//Build the time string
	strcpy(timeString, hours);
	strcat(timeString, ":");
	strcat(timeString, minutes);
	strcat(timeString, ":");
	strcat(timeString, seconds);
	
	return timeString;
}


/**
 * Returns the specified integer as a string because the designers of C were too 
 * distracted by their beards to write a lanague that doESNT SUCK ASS FUCK SHIT
 */
char *intToString(int dickens) {
	char * intString = (char*)malloc(10);
	itoa(dickens, intString,10);
	return intString;
}


/**
 * Creates fresh instances of all game objects and managers
 */
void loadGameObjects() {

	hge->System_Log("*****Loading Game Objects*****");

	hge->System_Log("Creating Player");
	if (thePlayer) delete thePlayer;
	thePlayer = new Player(saveManager->playerGridX, saveManager->playerGridY);

	hge->System_Log("Creating Enemy Manager");
	if (enemyManager) delete enemyManager;
	enemyManager = new EnemyManager();
	
	hge->System_Log("Creating Loot Manager");
	if (lootManager) delete lootManager;
	lootManager = new LootManager();
		
	hge->System_Log("Creating Projectile Manager");
	if (projectileManager) delete projectileManager;
	projectileManager = new ProjectileManager();
		
	hge->System_Log("Creating NPC Manager");
	if (npcManager) delete npcManager;
	npcManager = new NPCManager();

	hge->System_Log("Creating Enemy Group Manager");
	if (enemyGroupManager) delete enemyGroupManager;
	enemyGroupManager = new EnemyGroupManager();

	hge->System_Log("Creating Boss Manager");
	if (bossManager) delete bossManager;
	bossManager = new BossManager();

	hge->System_Log("Creating Window Manager");
	if (windowManager) delete windowManager;
	windowManager = new WindowManager();

	//Environment must be created last!
	hge->System_Log("Creating Environment");
	if (theEnvironment) delete theEnvironment;
	theEnvironment = new Environment();
	theEnvironment->loadArea(saveManager->currentArea, saveManager->currentArea, 
		saveManager->playerGridX, saveManager->playerGridY);

	hge->System_Log("******************************");

}

/**
 * Returns whether or not id is the id of an on cylinder switch
 */
bool isCylinderSwitchLeft(int id) {
	return (id == WHITE_SWITCH_LEFT || id == YELLOW_SWITCH_LEFT || id == GREEN_SWITCH_LEFT ||
		id == BLUE_SWITCH_LEFT || id == BROWN_SWITCH_LEFT || id == SILVER_SWITCH_LEFT);
}

/**
 * Returns whether or not id is the id of an off cylinder switch
 */
bool isCylinderSwitchRight(int id) {
	return (id == WHITE_SWITCH_RIGHT || id == YELLOW_SWITCH_RIGHT || id == GREEN_SWITCH_RIGHT ||
			id == BLUE_SWITCH_RIGHT || id == BROWN_SWITCH_RIGHT || id == SILVER_SWITCH_RIGHT);
}

/**
 * Returns whether or not id the id of a down cylinder
 */
bool isCylinderDown(int id) {
	return (id == WHITE_CYLINDER_DOWN || id == YELLOW_CYLINDER_DOWN || id == GREEN_CYLINDER_DOWN ||
			id == BLUE_CYLINDER_DOWN || id == BROWN_CYLINDER_DOWN || id == SILVER_CYLINDER_DOWN);
}

/**
 * Returns whether or not id the id of an up cylinder
 */
bool isCylinderUp(int id) {
	return (id == WHITE_CYLINDER_UP || id == YELLOW_CYLINDER_UP || id == GREEN_CYLINDER_UP ||
			id == BLUE_CYLINDER_UP|| id == BROWN_CYLINDER_UP || id == SILVER_CYLINDER_UP);
}

/**
 * Returns the largest of 2 floats.
 */
float maxFloat(float num1, float num2) {
	if (num1 > num2) return num1;
	else return num2;
}


/**
 * Shades the screen
 */
void shadeScreen(int alpha) {
	resources->GetSprite("blackScreen")->SetColor(ARGB(alpha,255,255,255));
	for (int i = 0; i < 35; i++) {
		for (int j = 0; j < 26; j++) {
			resources->GetSprite("blackScreen")->Render(i*30,j*30);
		}
	}
}

/**
 * Returns the grid x coordinate that x appears in
 */
int getGridX(int x) {
	return (x - x%64) / 64;
} 

/**
 * Returns the grid y coordinate that y appears in
 */
int getGridY(int y) {
	return (y - y%64) / 64;
}


/**
 * Returns the length of the straight line connecting (x,y) to the
 * player's position.
 */
int distanceFromPlayer(float x, float y) {
	int xDist = abs(x - thePlayer->x);
	int yDist = abs(y - thePlayer->y);

	return sqrt(float(xDist*xDist) + float(yDist*yDist));
}

/**
 * Returns the angle between (x1,y1) and (x2,y2)
 */
float getAngleBetween(int x1, int y1, int x2, int y2) {

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
 * Returns the amount of time that has passed since time
 */
float timePassedSince(float time) {
	return gameTime - time;
}

/**
 * Returns whether or not a collision layer id is a warp.
 */
bool isWarp(int id) {
	return (id == BLUE_WARP || id == RED_WARP || id == GREEN_WARP || YELLOW_WARP);
}


/**
 * Returns the parent area of the given area. Only the 5 parent areas have keys, so this method
 * is used to determine which of these 5 areas to save the key to!! The number returned is the
 * [area] index of Inventory.keys[area][key color] for the parent area.
 */
int getKeyIndex(int area) {
	switch (saveManager->currentArea) {
		case OLDE_TOWNE:
		case TUTS_TOMB:
		case SMOLDER_HOLLOW:
			return 0;
		case FOREST_OF_FUNGORIA:
			return 1;
		case SESSARIA_SNOWPLAINS:
			return 2;
		case WORLD_OF_DESPAIR:
			return 3;
		case CASTLE_OF_EVIL:
			return 4;
	}
}


/**
 * Rounds a float up to the nearest integer.
 */ 
int roundUp(float num) {
	if (num > (int)num) return (int)num + 1;
	else return (int)num;
}