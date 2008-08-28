/** 
 * GameData.cpp
 * 
 * Encapsulates all game data.
 */

#include "smiley.h"
#include "GameData.h"
#include "ProjectileManager.h"
#include "Player.h"
#include "hgestrings.h"
#include <string>

extern Player *thePlayer;
extern HGE *hge;

GameData::GameData() {
	loadEnemyData();
	loadAbilityData();
	gameText = new hgeStringTable("Data/GameText.dat");
}

GameData::~GameData() {

}

EnemyInfo GameData::getEnemyInfo(int enemyID) {
	return enemyInfo[enemyID];
}

Ability GameData::getAbilityInfo(int abilityID) {
	return abilities[abilityID];
}

std::list<EnemyName> GameData::getEnemyNames() {
	return enemyNameList;
}

const char *GameData::getGameText(const char *text) {
	return gameText->GetString(text);
}

const char *GameData::getAreaName(int area) {
	
	//Set zone specific info
	if (area == FOUNTAIN_AREA) {
		return "Smiley Town";
	} else if (area == OLDE_TOWNE) {
		return "Dunes of Salabia";
	} else if (area == SMOLDER_HOLLOW) {
		return "Smolder Hollow";
	} else if (area == FOREST_OF_FUNGORIA) {
		return "Forest of Fundoria";
	} else if (area == SESSARIA_SNOWPLAINS) {
		return "Sessaria Snowplains";
	} else if (area == TUTS_TOMB) {
		return "Tut's Tomb";
	} else if (area == WORLD_OF_DESPAIR) {
		return "Realm of Despair";
	} else if (area == CASTLE_OF_EVIL) {
		return "Castle Of Evil";
	} else if (area == SERPENTINE_PATH) {
		return "The Serpentine Path";
	}
	
}

////////// Private functions //////////////////


void GameData::loadAbilityData() {

	//Clinton's Cane
	strcpy(abilities[CANE].name, "Cane Of Clinton");
	strcpy(abilities[CANE].description, 
		"Use for 3 seconds to communicate telepathically with Bill Clinton. \n\nMana Cost: 10");
	abilities[CANE].type = ACTIVATED;
	abilities[CANE].manaCost = 10;

	//Jesus' Sandals
    strcpy(abilities[WATER_BOOTS].name, "Jesus' Sandals");
	strcpy(abilities[WATER_BOOTS].description, 
		"While equipped you gain the power of Jesus Christ. (That means you can walk on water)");
	abilities[WATER_BOOTS].type = HOLD;
	abilities[WATER_BOOTS].manaCost = 0;

	//Boots of 14
	strcpy(abilities[SPRINT_BOOTS].name, "Speed Boots");
	strcpy(abilities[SPRINT_BOOTS].description, "When activated you run 75% faster.");
	abilities[SPRINT_BOOTS].type = ACTIVATED;
	abilities[SPRINT_BOOTS].manaCost = 0;

	//Fire Breath
	strcpy(abilities[FIRE_BREATH].name, "Fire Breath");
	strcpy(abilities[FIRE_BREATH].description, 
		"Allows you to breath deadly fire breath.\n\nMana Cost: 10/second\n");
	strcat(abilities[FIRE_BREATH].description, "Damage: ");
	strcat(abilities[FIRE_BREATH].description, intToString(thePlayer->getFireBreathDamage()));
	strcat(abilities[FIRE_BREATH].description, " per second.");
	abilities[FIRE_BREATH].type = HOLD;
	abilities[FIRE_BREATH].manaCost = 10;

	//Ice Breath
	strcpy(abilities[ICE_BREATH].name, "Ice Breath");
	strcpy(abilities[ICE_BREATH].description, 
		"Unleashes an icy blast that can freeze enemies.\n\n\nMana Cost: 10");
	abilities[ICE_BREATH].type = ACTIVATED;
	abilities[ICE_BREATH].manaCost = 10;

	//Reflection Shield
	strcpy(abilities[REFLECTION_SHIELD].name, "Reflection Shield");
	strcpy(abilities[REFLECTION_SHIELD].description, 
		"Activate to deflect certain projectiles.\n\n\nMana Cost: 10");
	abilities[REFLECTION_SHIELD].type = HOLD;
	abilities[REFLECTION_SHIELD].manaCost = 10;

	//Hover
	strcpy(abilities[HOVER].name, "Hover");
	strcpy(abilities[HOVER].description, 
		"Grants you the power to use hover pads\n\n\nMana Cost: 15/second");
	abilities[HOVER].type = HOLD;
	abilities[HOVER].manaCost = 15;

	//Lightning Orbs
	strcpy(abilities[LIGHTNING_ORB].name, "Lightning Orbs");
	strcpy(abilities[LIGHTNING_ORB].description, 
		"Shoots orbs of lightning. \n\n\nMana Cost: 5\n");
	strcat(abilities[LIGHTNING_ORB].description, "Damage: ");
	strcat(abilities[LIGHTNING_ORB].description, intToString(thePlayer->getLightningOrbDamage()));
	abilities[LIGHTNING_ORB].type = ACTIVATED;
	abilities[LIGHTNING_ORB].manaCost = 5;

	//Shrink
	strcpy(abilities[SHRINK].name, "Shrink");
	strcpy(abilities[SHRINK].description, "When activated Smiley will shrink in size and be able to fit into smaller spaces.");
	abilities[SHRINK].type = ACTIVATED;
	abilities[SHRINK].manaCost = 0;

	//Silly Pad
	strcpy(abilities[SILLY_PAD].name, "Silly Pad");
	strcpy(abilities[SILLY_PAD].description, 
		"Places a Silly Pad. They are so silly that enemies can't even cross them!\n\nMana Cost: 5");
	abilities[SILLY_PAD].type = ACTIVATED;
	abilities[SILLY_PAD].manaCost = 5;

	//King Tut's Mask
	strcpy(abilities[TUTS_MASK].name, "Tut's Mask");
	strcpy(abilities[TUTS_MASK].description, 
		"Grants the wearer the power of invisibility.\n\n\nMana Cost: 25/second");
	abilities[TUTS_MASK].type = HOLD;
	abilities[TUTS_MASK].manaCost = 25;

	//Frisbee
	strcpy(abilities[FRISBEE].name, "Frisbee!?!?! K");
	strcpy(abilities[FRISBEE].description, "Throws a frisbee that can stun enemies.");
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
		hge->System_Log("name");
		//Enemy name
		varName = intToString(i);
		varName += "Name";
		if (enemyStringTable->GetString(varName.c_str()) != 0) {
			addEnemyName(i, enemyStringTable->GetString(varName.c_str()));
		}
		hge->System_Log("one graphic");
		//Has one graphic?
		varName = intToString(i);
		varName += "OneGraphic";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].hasOneGraphic = false;
		else enemyInfo[i].hasOneGraphic = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
		hge->System_Log("column");
		//Graphics column
		varName = intToString(i);
		varName += "GCol";
		enemyInfo[i].gCol = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("row");
		//Graphics row
		varName = intToString(i);
		varName += "GRow";
		enemyInfo[i].gRow = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("frames");
		//Number animation frames (default 1)
		varName = intToString(i);
		varName += "NumFrames";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].numFrames = 1;
		else enemyInfo[i].numFrames = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("type");
		//Enemy Type
		varName = intToString(i);
		varName += "EnemyType";
		enemyInfo[i].enemyType = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("wander");
		//WanderType
		varName = intToString(i);
		varName += "WanderType";
		enemyInfo[i].wanderType = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("hp");
		//HP
		varName = intToString(i);
		varName += "HP";
		enemyInfo[i].hp = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("damage");
		//Damage
		varName = intToString(i);
		varName += "Damage";
		enemyInfo[i].damage = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("speed");
		//Speed
		varName = intToString(i);
		varName += "Speed";
		enemyInfo[i].speed = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("radius");
		//Radius
		varName = intToString(i);
		varName += "Radius";
		enemyInfo[i].radius = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("land");
		//Can walk on Land
		varName = intToString(i);
		varName += "Land";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].land = false;
		else enemyInfo[i].land = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("lava");
		//Can walk on Lava
		varName = intToString(i);
		varName += "Lava";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].lava = false;
		else enemyInfo[i].lava = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("mushroom");
		//Can walk on Mushrooms
		varName = intToString(i);
		varName += "Mushrooms";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].mushrooms = false;
		else enemyInfo[i].mushrooms = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("swater");
		//Can walk on Shallow Water
		varName = intToString(i);
		varName += "SWater";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].shallowWater = false;
		else enemyInfo[i].shallowWater = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("dwater");
		//Can walk on Deep Water
		varName = intToString(i);
		varName += "DWater";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].deepWater = false;
		else enemyInfo[i].deepWater = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("slime");
		//Can walk on Slime
		varName = intToString(i);
		varName += "Slime";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].slime = false;
		else enemyInfo[i].slime = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("tongue");
		//Immune to smiley's tongue
		varName = intToString(i);
		varName += "ImmuneTongue";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToTongue = false;
		else enemyInfo[i].immuneToTongue = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("fire");
		//Immune to fire
		varName = intToString(i);
		varName += "ImmuneFire";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToFire = false;
		else enemyInfo[i].immuneToFire = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("lightning");
		//Immune to lightning
		varName = intToString(i);
		varName += "ImmuneLightning";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToLightning = false;
		else enemyInfo[i].immuneToLightning = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("stun");
		//Immune to stun
		varName = intToString(i);
		varName += "ImmuneStun";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToStun = false;
		else enemyInfo[i].immuneToStun = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("freeze");
		//Immune to freeze
		varName = intToString(i);
		varName += "ImmuneFreeze";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].immuneToFreeze = false;
		else enemyInfo[i].immuneToFreeze = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("invincible");
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
hge->System_Log("v1");
		//Variable 1
		varName = intToString(i);
		varName += "Variable1";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].variable1 = 0;
		else enemyInfo[i].variable1 = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("v2");
		//Variable 2
		varName = intToString(i);
		varName += "Variable2";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].variable2 = 0;
		else enemyInfo[i].variable2 = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("ranged");
		//Has a ranged attack
		varName = intToString(i);
		varName += "HasRanged";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].hasRangedAttack = false;
		else enemyInfo[i].hasRangedAttack = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);
hge->System_Log("chase");
		//Will chase smiley
		varName = intToString(i);
		varName += "Chases";
		if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].chases = false;
		else enemyInfo[i].chases = (strcmp(enemyStringTable->GetString(varName.c_str()), "T") == 0);

		//Load ranged info for ranged enemies
		if (enemyInfo[i].hasRangedAttack) {
hge->System_Log("type");
			//Ranged Attack Type
			varName = intToString(i);
			varName += "PType";
			if (enemyStringTable->GetString(varName.c_str()) == 0) enemyInfo[i].rangedType = PROJECTILE_1;
			else enemyInfo[i].rangedType = atoi(enemyStringTable->GetString(varName.c_str()));
hge->System_Log("range");
			//Ranged Attack Range
			strcpy(param, num);
			strcat(param, "Range");
			enemyInfo[i].range = atoi(enemyStringTable->GetString(param));
hge->System_Log("delay");
			//Ranged Attack Delay
			strcpy(param, num);
			strcat(param, "Delay");
			enemyInfo[i].delay = atoi(enemyStringTable->GetString(param));
hge->System_Log("speed");
			//Projectile Speed
			strcpy(param, num);
			strcat(param, "PSpeed");
			enemyInfo[i].projectileSpeed = atoi(enemyStringTable->GetString(param));
hge->System_Log("damage");
			//Projectile Damage
			strcpy(param, num);
			strcat(param, "PDamage");
			enemyInfo[i].projectileDamage = (float)atoi(enemyStringTable->GetString(param)) / 100.0;
		}
hge->System_Log("Nigger");
	}

} // end loadEnemyData()

/**
 * Adds an enemy name to the list of enemy names if it doesn't already exist.
 */
void GameData::addEnemyName(int id, std::string name) {
	
	bool alreadyExists = false;
	
	std::list<EnemyName>::iterator i;
	for (i = enemyNameList.begin(); i != enemyNameList.end(); i++) {
		if (strcmp(name.c_str(), i->name.c_str()) == 0) {
			alreadyExists = true;
		}
	}

	if (!alreadyExists) {
		EnemyName enemyName;
		enemyName.name = name;
		enemyName.id = id;
		enemyNameList.push_back(enemyName);
	}

}