/** 
 * GameData.cpp
 * 
 * Encapsulates all game data.
 */

#include "GameData.h"
#include "Projectiles.h"
#include <string>

GameData::GameData() {
	loadEnemyData();
	loadAbilityData();
}

GameData::~GameData() {

}

EnemyInfo GameData::getEnemyInfo(int enemyID) {
	return enemyInfo[enemyID];
}

Ability GameData::getAbilityInfo(int abilityID) {
	return abilities[abilityID];
}

char *GameData::getGameText(const char *text) {
	return gameText->GetString(text);
}

////////// Private functions //////////////////


void GameData::loadAbilityData() {

	//Smiley's Cane
	strcpy(abilities[CANE].description, "Mysterious Cane");
	abilities[CANE].type = ACTIVATED;
	abilities[CANE].manaCost = 10;

	//Jesus' Sandals
    strcpy(abilities[WATER_BOOTS].description, "Jesus' Sandals");
	abilities[WATER_BOOTS].type = HOLD;
	abilities[WATER_BOOTS].manaCost = 0;

	//Boots of 14
	strcpy(abilities[SPRINT_BOOTS].description, "Speed Boots");
	abilities[SPRINT_BOOTS].type = ACTIVATED;
	abilities[SPRINT_BOOTS].manaCost = 0;

	//Fire Breath
	strcpy(abilities[FIRE_BREATH].description, "Fire Breath");
	abilities[FIRE_BREATH].type = HOLD;
	abilities[FIRE_BREATH].manaCost = 10;

	//Ice Breath
	strcpy(abilities[ICE_BREATH].description, "Ice Breath");
	abilities[ICE_BREATH].type = ACTIVATED;
	abilities[ICE_BREATH].manaCost = 10;

	//Reflection Shield
	strcpy(abilities[REFLECTION_SHIELD].description, "Reflection Shield");
	abilities[REFLECTION_SHIELD].type = HOLD;
	abilities[REFLECTION_SHIELD].manaCost = 20;

	//Wings
	strcpy(abilities[HOVER].description, "Hover");
	abilities[HOVER].type = HOLD;
	abilities[HOVER].manaCost = 15;

	//Lightning Orbs
	strcpy(abilities[LIGHTNING_ORB].description, "Lightning Orbs");
	abilities[LIGHTNING_ORB].type = ACTIVATED;
	abilities[LIGHTNING_ORB].manaCost = 1;

	//Shrink
	strcpy(abilities[SHRINK].description, "Shrink");
	abilities[SHRINK].type = ACTIVATED;
	abilities[SHRINK].manaCost = 0;

	//Silly Pad
	strcpy(abilities[SILLY_PAD].description, "Silly Pad");
	abilities[SILLY_PAD].type = ACTIVATED;
	abilities[SILLY_PAD].manaCost = 3;

	//King Tut's Mask
	strcpy(abilities[TUTS_MASK].description, "Tut's Mask");
	abilities[TUTS_MASK].type = HOLD;
	abilities[TUTS_MASK].manaCost = 25;

	//Frisbee
	strcpy(abilities[FRISBEE].description, "Frisbee!?!?! K");
	abilities[FRISBEE].type = ACTIVATED;
	abilities[FRISBEE].manaCost = 0;


}

/**
 * Loads enemy data from file "Enemy.dat"
 */
void GameData::loadEnemyData() {

	char num[2];
	char param[68];
	std::string varName;
	
	enemyStringTable = new hgeStringTable("Data/Enemies.dat");
	int numEnemies = atoi(enemyStringTable->GetString("numEnemies"));

	for (int i = 0; i < numEnemies; i++) {
		itoa(i, num, 10);

		//Graphics column
		varName = intToString(i);
		varName += "GCol";
		enemyInfo[i].gCol = atoi(enemyStringTable->GetString(varName.c_str()));

		//Graphics row
		varName = intToString(i);
		varName += "GRow";
		enemyInfo[i].gRow = atoi(enemyStringTable->GetString(varName.c_str()));

		//Enemy Type
		varName = intToString(i);
		varName += "EnemyType";
		enemyInfo[i].enemyType = atoi(enemyStringTable->GetString(varName.c_str()));
		
		//WanderType
		varName = intToString(i);
		varName += "WanderType";
		enemyInfo[i].wanderType = atoi(enemyStringTable->GetString(varName.c_str()));
		
		//HP
		varName = intToString(i);
		varName += "HP";
		enemyInfo[i].hp = atoi(enemyStringTable->GetString(varName.c_str()));
		
		//Damage
		varName = intToString(i);
		varName += "Damage";
		enemyInfo[i].damage = atoi(enemyStringTable->GetString(varName.c_str()));
		
		//Speed
		varName = intToString(i);
		varName += "Speed";
		enemyInfo[i].speed = atoi(enemyStringTable->GetString(varName.c_str()));
		
		//Radius
		varName = intToString(i);
		varName += "Radius";
		enemyInfo[i].radius = atoi(enemyStringTable->GetString(varName.c_str()));
		
		//Can walk on Land
		varName = intToString(i);
		varName += "Land";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].land = false;
		else enemyInfo[i].land = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		
		//Can walk on Lava
		varName = intToString(i);
		varName += "Lava";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].lava = false;
		else enemyInfo[i].lava = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		
		//Can walk on Shallow Water
		varName = intToString(i);
		varName += "SWater";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].shallowWater = false;
		else enemyInfo[i].shallowWater = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		
		//Can walk on Deep Water
		varName = intToString(i);
		varName += "DWater";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].deepWater = false;
		else enemyInfo[i].deepWater = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		
		//Can walk on Slime
		varName = intToString(i);
		varName += "Slime";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].slime = false;
		else enemyInfo[i].slime = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
	
		//Immune to smiley's tongue
		varName = intToString(i);
		varName += "ImmuneTongue";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToTongue = false;
		else enemyInfo[i].immuneToTongue = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);

		//Immune to fire
		varName = intToString(i);
		varName += "ImmuneFire";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToFire = false;
		else enemyInfo[i].immuneToFire = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);

		//Immune to lightning
		varName = intToString(i);
		varName += "ImmuneLightning";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToLightning = false;
		else enemyInfo[i].immuneToLightning = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		
		//Immune to stun
		varName = intToString(i);
		varName += "ImmuneStun";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToStun = false;
		else enemyInfo[i].immuneToStun = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
			
		//Immune to freeze
		varName = intToString(i);
		varName += "ImmuneFreeze";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToFreeze = false;
		else enemyInfo[i].immuneToFreeze = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);

		//Invincible
		varName = intToString(i);
		varName += "Invincible";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].invincible = false;
		else {
			enemyInfo[i].invincible = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
			if (enemyInfo[i].invincible) {
				enemyInfo[i].immuneToFire = enemyInfo[i].immuneToFreeze = enemyInfo[i].immuneToLightning = enemyInfo[i].immuneToStun = enemyInfo[i].immuneToTongue = true;
			}
		}

		//Variable 1
		varName = intToString(i);
		varName += "Variable1";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].variable1 = 0;
		else enemyInfo[i].variable1 = atoi(enemyStringTable->GetString(varName.c_str()));

		//Variable 2
		varName = intToString(i);
		varName += "Variable2";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].variable2 = 0;
		else enemyInfo[i].variable2 = atoi(enemyStringTable->GetString(varName.c_str()));
					
		//Has a ranged attack
		varName = intToString(i);
		varName += "HasRanged";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].hasRangedAttack = false;
		else enemyInfo[i].hasRangedAttack = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
			
		//Will chase smiley
		varName = intToString(i);
		varName += "Chases";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].chases = false;
		else enemyInfo[i].chases = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		
		//Load ranged info for ranged enemies
		if (enemyInfo[i].hasRangedAttack) {
			//Ranged Attack Type
			varName = intToString(i);
			varName += "PType";
			if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].rangedType = PROJECTILE_1;
			else enemyInfo[i].rangedType = atoi(enemyStringTable->GetString(varName.c_str()));
			//Ranged Attack Range
			strcpy(param, num);
			strcat(param, "Range");
			enemyInfo[i].range = atoi(enemyStringTable->GetString(param));
			//Ranged Attack Delay
			strcpy(param, num);
			strcat(param, "Delay");
			enemyInfo[i].delay = atoi(enemyStringTable->GetString(param));
			//Projectile Speed
			strcpy(param, num);
			strcat(param, "PSpeed");
			enemyInfo[i].projectileSpeed = atoi(enemyStringTable->GetString(param));
			//Projectile Damage
			strcpy(param, num);
			strcat(param, "PDamage");
			enemyInfo[i].projectileDamage = (float)atoi(enemyStringTable->GetString(param)) / 100.0;
		}
			
	}

} // end loadEnemyData()