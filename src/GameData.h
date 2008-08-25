#ifndef GAMEDATA_H
#define GAMEDATA_H

#include <string>
#include <list>

class hgeStringTable;

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

struct EnemyName {
	int id;
	std::string name;
};

struct Ability {
	char name[32];
	char description[128];
	int manaCost;
	int type;
};

class GameData {

public:

	GameData();
	~GameData();

	EnemyInfo getEnemyInfo(int enemyID);
	Ability getAbilityInfo(int abilityID);
	std::list<EnemyName> getEnemyNames();
	const char *getGameText(const char *text);
	const char *getAreaName(int area);


private:

	void loadEnemyData();
	void loadAbilityData();
	void addEnemyName(int id, std::string name);

	hgeStringTable *enemyStringTable;
	EnemyInfo enemyInfo[99];
	Ability abilities[16];
	std::list<EnemyName> enemyNameList;
	hgeStringTable *gameText;

};


#endif