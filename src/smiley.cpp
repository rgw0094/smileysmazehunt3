/**
 * Smiley's Maze Hunt 3 global functions
 */
#include "SMH.h"
#include "smiley.h"
#include "environment.h"
#include "player.h"
#include "hgerect.h"
#include "boss.h"

extern int gameState;
extern hgeResourceManager *resources;
extern int frameCounter;

extern SMH *smh;
extern HGE *hge;

/**
 * Switches the game state
 */
void enterGameState(int newState) {

	//If leaving the menu, clear all the menu resources
	if (gameState == MENU) {	
		resources->Purge(RES_MENU);
	}

	//If leaving game state save the player's playing time
	if (gameState == GAME) {
		smh->saveManager->saveTimePlayed();
	}

	gameState = newState;

	//Entering game state
	if (gameState == GAME) {
		smh->setGameTime(0.0);
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

	int x1 = getScreenX(box->x1);
	int x2 = getScreenX(box->x2);
	int y1 = getScreenY(box->y1);
	int y2 = getScreenY(box->y2);

	hge->Gfx_RenderLine(x1, y1, x2, y1, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x2, y1, x2, y2, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x2, y2, x1, y2, ARGB(255,r,g,b));
	hge->Gfx_RenderLine(x1, y2, x1, y1, ARGB(255,r,g,b));

}

/**
 * Return whether a grid coordinate is in bounds or not
 */
bool inBounds(int gridX, int gridY) {
	return (gridX >= 0 && gridY >= 0 && gridX < smh->environment->areaWidth && gridY < smh->environment->areaHeight);
}

/**
 * Returns the screen x position given the global x position
 */
int getScreenX(int x) {
	return x - smh->environment->xGridOffset*64.0 - smh->environment->xOffset;
}

/**
 * Returns the screen y position given the global y position
 */
int getScreenY(int y) {
	return y - smh->environment->yGridOffset*64.0 - smh->environment->yOffset;										  
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
const char *getTimeString(int time) {

	std::string timeString;

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
	timeString = hours;
	timeString += ":";
	timeString += minutes;
	timeString += ":";
	timeString += seconds;
	return timeString.c_str();
}

/**
 * Returns the specified integer as a string because the designers of C were too 
 * distracted by their beards to write a language that doESNT SUCK ASS FUCK SHIT
 */
std::string intToString(int n) {
	std::string numberString = "";
	char number[10];
	itoa(n, number, 10);
	numberString += number;
	return numberString;
}

/**
 * Returns the int as a string, with the given number of digits
 */
std::string intToString(int number, int digits) {
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

float maxFloat(float f1, float f2, float f3, float f4) {
	if (f1 > f2 && f1 > f3 && f1 > f4) {
		return f1;
	} else if (f2 > f1 && f2 > f3 && f2 > f4) {
		return f2;
	} else if (f3 > f1 && f3 > f2 && f3 > f4) {
		return f3;
	} else if (f4 > f4 && f4 > f2 && f4 > f3) {
		return f4;
	}
}

/**
 * Returns the direction to face if it should face the direction it is moving the fastest.
 */
int getFacingDirection(float dx, float dy) {
	if (abs(dx) > abs (dy)) {
		return dx > 0 ? RIGHT : LEFT;
	} else if(abs(dy) > abs(dy)) {
		return dy > 0 ? DOWN : UP;
	}
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
	int xDist = abs(x - smh->player->x);
	int yDist = abs(y - smh->player->y);

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
	return smh->getGameTime() - time;
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
 * [area] index of SaveManager.numKeys[area][key color] for the parent area.
 */
int getKeyIndex(int area) {
	switch (smh->saveManager->currentArea) {
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

/**
 * Returns the current hint number based on what boss has been killed.
 */
int getCurrentHint() {
	if (smh->saveManager->isBossKilled(FIRE_BOSS2)) {
		return 7;
	} else if (smh->saveManager->isBossKilled(MUSHROOM_BOSS)) {
		return 6;
	} else if (smh->saveManager->isBossKilled(DESPAIR_BOSS)) {
		return 5;
	} else if (smh->saveManager->isBossKilled(DESERT_BOSS)) {
		return 4;
	} else if (smh->saveManager->isBossKilled(FOREST_BOSS)) {
		return 3;
	} else if (smh->saveManager->isBossKilled(SNOW_BOSS)) {
		return 2;
	} else if (smh->saveManager->isBossKilled(FIRE_BOSS)) {
		return 1;
	} else {
		return 0;
	}

}