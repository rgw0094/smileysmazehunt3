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
	currentBoss = NULL;
}

BossManager::~BossManager() {
	if (currentBoss) delete currentBoss;
}

/** 
 * Creates an instance of the specified boss.
 */
void BossManager::spawnBoss(int bossID, int groupID, int gridX, int gridY) {

	hge->System_Log("Spawning Boss %d", bossID);

	//Delete old boss just in case
	if (currentBoss) delete currentBoss;

	//Create the boss object based on the bossID
	if (bossID == FIRE_BOSS) {
		currentBoss = new FireBoss(gridX,gridY, groupID);
	} else if (bossID == FOREST_BOSS) {
		currentBoss = new ForestBoss(gridX,gridY, groupID);
	} else if (bossID == SNOW_BOSS) {
		currentBoss = new SnowBoss(gridX, gridY, groupID);
	} else if (bossID == DESERT_BOSS) {
		currentBoss = new DesertBoss(gridX, gridY, groupID);
	} else if (bossID == MUSHROOM_BOSS) {
		currentBoss = new MushroomBoss(gridX, gridY, groupID);
	} else if (bossID == DESPAIR_BOSS) {
		currentBoss = new DespairBoss(gridX, gridY, groupID);
	} else if (bossID == FIRE_BOSS2) {
		currentBoss = new FireBossTwo(gridX, gridY, groupID);
	} else {
		//Unimplemented boss - exit the program
		hge->System_Log("FATAL ERROR: BossManager.spawnBoss() received invalid boss ID!!!");
		exit(1);
	}
	
	//Register the boss in its bossID group
	enemyGroupManager->addEnemy(groupID);

}

/**
 * Update method called every frame.
 */
void BossManager::update(float dt) {
	if (currentBoss) {
		if (currentBoss->update(dt)) {
			delete currentBoss;
			currentBoss = NULL;
		}
	}
}

/**
 * Draw method called before Smiley is drawn.
 */
void BossManager::drawBeforeSmiley(float dt) {
	if (currentBoss) {
		currentBoss->draw(dt);
	}
}

/**
 * Draw method called after Smiley is drawn.
 */
void BossManager::drawAfterSmiley(float dt) {
	if (currentBoss) {
		currentBoss->drawAfterSmiley(dt);
	}
}