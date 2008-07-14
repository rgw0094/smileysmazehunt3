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

class GameData {

public:

	GameData();
	~GameData();

	EnemyInfo getEnemyInfo(int enemyID);


private:

	void loadEnemyData();

	hgeStringTable *enemyStringTable;
	EnemyInfo enemyInfo[99];

};


#endif