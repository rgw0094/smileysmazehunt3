#ifndef _SMILEY_H_
#define _SMILEY_H_

#include <stdlib.h>
#include "hgerect.h"

//Constants
#define PI 3.14159265357989232684

//Gameplay values
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define NUM_NPCS 99
#define PLAYER_WIDTH 61
#define PLAYER_HEIGHT 72

//Resource Groups
#define RES_MENU 10
#define RES_PHYREBOZZ 100
#define RES_PORTLYPENGUIN 101
#define RES_GARMBORN 102
#define RES_CORNWALLIS 103
#define RES_CALYPSO 105

#define NUM_BOSSES 12

//Hardcoded IDs
#define DRAW_AFTER_SMILEY 990

//Abilities
#define NUM_ABILITIES 12
#define NO_ABILITY -1
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
#define NUM_AREAS 9
#define FOUNTAIN_AREA 0
#define OLDE_TOWNE 1
#define TUTS_TOMB 2
#define FOREST_OF_FUNGORIA 3
#define SESSARIA_SNOWPLAINS 4
#define WORLD_OF_DESPAIR 5
#define SERPENTINE_PATH 6
#define CASTLE_OF_EVIL 7
#define SMOLDER_HOLLOW 8

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

struct Point {
	int x, y;
};

//Global functions
void enterGameState(int newState);
bool inBounds(int gridX, int gridY);
void drawCollisionBox(hgeRect *box, int color);
int getScreenX(int x);
int getScreenY(int y);
int getGridX(int x);
int getGridY(int y);
int distance(int x1, int y1, int x2, int y2);
const char *getTimeString(int seconds);
const char *intToString(int dickens);
const char *intToString(int dickens, int numDigits);
bool isCylinderSwitchLeft(int id);
bool isCylinderSwitchRight(int id);
bool isCylinderUp(int id);
bool isCylinderDown(int id);
bool isWarp(int id);
float maxFloat(float num1, float num2);
float maxFloat(float f1, float f2, float f3, float f4);
int getFacingDirection(float dx, float dy);
void shadeScreen(int alpha);
float getAngleBetween(int x1, int y1, int x2, int y2);
float timePassedSince(float time);
int getKeyIndex(int area);
void loadGameObjects();
void initAbilities();
int roundUp(float num);
int getCurrentHint();

#endif
