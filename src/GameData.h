#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "hgestrings.h"

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

struct Ability {
	char description[68];
	int manaCost;
	int type;
};

class GameData {

public:

	GameData();
	~GameData();

	EnemyInfo getEnemyInfo(int enemyID);
	Ability getAbilityInfo(int abilityID);
	char *getGameText(const char *text);
	char *getAreaName(int area);


private:

	void loadEnemyData();
	void loadAbilityData();

	hgeStringTable *enemyStringTable;
	EnemyInfo enemyInfo[99];
	Ability abilities[16];
	hgeStringTable *gameText;

};


#endif