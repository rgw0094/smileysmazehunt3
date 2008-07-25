#include "boss.h"
#include "fireboss.h"
#include "DesertBoss.h"
#include "SnowBoss.h"
#include "ForestBoss.h"
#include "MushroomBoss.h"
#include "DespairBoss.h"
#include "Fireboss2.h"
#include "EnemyGroupManager.h"
#include "hge.h"

extern EnemyGroupManager *enemyGroupManager;
extern HGE *hge;

BossManager::BossManager() {
}

BossManager::~BossManager() {
	reset();
}

/** 
 * Creates an instance of the specified boss.
 */
void BossManager::spawnBoss(int bossID, int groupID, int gridX, int gridY) {

	hge->System_Log("Spawning Boss %d", bossID);

	BossStruct newBoss;

	//Create the boss object based on the bossID
	if (bossID == FIRE_BOSS) {
		newBoss.boss = new FireBoss(gridX,gridY, groupID);
	} else if (bossID == FOREST_BOSS) {
		newBoss.boss = new ForestBoss(gridX,gridY, groupID);
	} else if (bossID == SNOW_BOSS) {
		newBoss.boss = new SnowBoss(gridX, gridY, groupID);
	} else if (bossID == DESERT_BOSS) {
		newBoss.boss = new DesertBoss(gridX, gridY, groupID);
	} else if (bossID == MUSHROOM_BOSS) {
		newBoss.boss = new MushroomBoss(gridX, gridY, groupID);
	} else if (bossID == DESPAIR_BOSS) {
		newBoss.boss = new DespairBoss(gridX, gridY, groupID);
	} else if (bossID == FIRE_BOSS2) {
		newBoss.boss = new FireBossTwo(gridX, gridY, groupID);
	} else {
		//Unimplemented boss - exit the program
		hge->System_Log("FATAL ERROR: BossManager.spawnBoss() received invalid boss ID!!!");
		exit(1);
	}

	bossList.push_back(newBoss);
	
	//Register the boss in its bossID group
	enemyGroupManager->addEnemy(groupID);

}

/**
 * Update method called every frame.
 */
void BossManager::update(float dt) {

	std::list<BossStruct>::iterator i;
	for (i = bossList.begin(); i != bossList.end(); i++) {
		if (i->boss->update(dt)) {
			delete i->boss;
			i = bossList.erase(i);
		}
	}

}

/**
 * Draw method called before Smiley is drawn.
 */
void BossManager::drawBeforeSmiley(float dt) {
std::list<BossStruct>::iterator i;
	for (i = bossList.begin(); i != bossList.end(); i++) {
		i->boss->draw(dt);
	}
}

/**
 * Draw method called after Smiley is drawn.
 */
void BossManager::drawAfterSmiley(float dt) {
	std::list<BossStruct>::iterator i;
	for (i = bossList.begin(); i != bossList.end(); i++) {
		i->boss->drawAfterSmiley(dt);
	}
}

/**
 * Deletes all managed bosses.
 */
void BossManager::reset() {
	std::list<BossStruct>::iterator i;
	for (i = bossList.begin(); i != bossList.end(); i++) {
		delete i->boss;
		i = bossList.erase(i);
	}
	bossList.clear();
}