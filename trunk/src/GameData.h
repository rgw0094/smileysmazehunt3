#ifndef GAMEDATA_H
#define GAMEDATA_H

#include "hge include/hgestrings.h"

/**
 * Stores info for each enemy id
 */ 
struct EnemyInfo {

	//For all enemies
	int gRow, gCol;
	int enemyType, wanderType, hp, speed, radius, damage, rangedType;
	bool land, shallowWater, deepWater, slime, lava;
	bool immuneToFire, immuneToTongue, immuneToLightning, immuneToStun, immuneToFreeze, invincible;
	int variable1, variable2;

	//For basic enemies
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


private:

	void loadEnemyData();
	void loadAbilityData();

	hgeStringTable *enemyStringTable;
	EnemyInfo enemyInfo[99];
	Ability abilities[16];
	hgeStringTable *gameText;

};


#endif