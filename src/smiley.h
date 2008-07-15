#include <list>
#include <stdio.h>
#include <iostream>
#ifndef _SMILEY_H_
#define _SMILEY_H_

#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <string>
//#include < string >
#include < sstream >
#include < iostream >

#include "hge include/hgeresource.h"
#include "hge include/hgesprite.h"
#include "hge include/hgeanim.h"

//Constants
#define PI 3.14159265357989232684

//Gameplay values
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define NUM_NPCS 99
#define PLAYER_WIDTH 61
#define PLAYER_HEIGHT 72
#define ITEM_SIZE 64
#define SILLY_PAD_TIME 30		//Number of seconds silly pads stay active
#define JESUS_SANDLE_TIME 3.3	//Number of second Jesus' sandles stay active 
								//before Smiley loses FAITH

//Resource Groups
#define RES_MENU 10
#define RES_PHYREBOZZ 100
#define RES_PORTLYPENGUIN 101
#define RES_GARMBORN 102
#define RES_CORNWALLIS 103
#define RES_CALYPSO 105

//Boss Text
#define TEXT_FIREBOSS_INTRO 100
#define TEXT_FIREBOSS_VICTORY 101

#define TEXT_DESERTBOSS_INTRO 130
#define TEXT_DESERTBOSS_VICTORY 131

//Bosses(for enemy layer)
#define NUM_BOSSES 12
#define FIRE_BOSS 240
#define DESERT_BOSS 241
#define SNOW_BOSS 242
#define FOREST_BOSS 243
#define MUSHROOM_BOSS 244
#define DESPAIR_BOSS 245

//Hardcoded IDs
#define DRAW_AFTER_SMILEY 990

//Abilities
#define NUM_ABILITIES 12
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

//Enemy Types
#define ENEMY_BASIC 0
#define ENEMY_EVIL_EYE 1
#define ENEMY_GUMDROP 2
#define ENEMY_BOMB_GENERATOR 3
#define ENEMY_CHARGER 4
#define ENEMY_CLOWNCRAB 5
#define ENEMY_BATLET_DIST 6
#define ENEMY_BUZZARD 7
#define ENEMY_SAD_SHOOTER 8
#define ENEMY_FLOATER 9
#define ENEMY_FLAILER 10
#define ENEMY_TENTACLE 11
#define ENEMY_TURRET 12
#define ENEMY_GHOST 13
#define ENEMY_FAKE 14

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
#define LOADING_LEVEL_PHASE1 30
#define LOADING_LEVEL_PHASE2 31

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


//-------- Collision Layer ---------//
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
//...
#define WHITE_CYLINDER_UP 58
#define YELLOW_CYLINDER_UP 59
#define GREEN_CYLINDER_UP 60
#define BLUE_CYLINDER_UP 61
#define BROWN_CYLINDER_UP 62
#define SILVER_CYLINDER_UP 63
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

//Colors
#define RED 0
#define YELLOW 1
#define GREEN 2
#define BLUE 3





struct Point {
	int x, int y;
};

//Global functions
void enterGameState(int newState);
bool inBounds(int gridX, int gridY);
void drawCollisionBox(hgeRect *box, int color);
void setTerrainCollisionBox(hgeRect *box, int whatFor, int gridX, int gridY);
int getScreenX(int x);
int getScreenY(int y);
int getGridX(int x);
int getGridY(int y);
int distance(int x1, int y1, int x2, int y2);
char *getTime(int seconds);
char *intToString(int dickens);
bool isCylinderSwitchLeft(int id);
bool isCylinderSwitchRight(int id);
bool isCylinderUp(int id);
bool isCylinderDown(int id);
bool isWarp(int id);
float maxFloat(float num1, float num2);
void shadeScreen(int alpha);
float getAngleBetween(int x1, int y1, int x2, int y2);
float timePassedSince(float time);
int getKeyIndex(int area);
void loadGameObjects();
void initAbilities();
int roundUp(float num);

#endif
