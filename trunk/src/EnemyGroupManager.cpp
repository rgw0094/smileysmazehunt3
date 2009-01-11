#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "Environment.h"
#include "Player.h"
#include "hgeresource.h"

extern SMH *smh;

EnemyGroupManager::EnemyGroupManager() {
	//Init enemy groups
	resetGroups();
}

EnemyGroupManager::~EnemyGroupManager() {
	
}

/**
 * Resets all enemy groups
 */
void EnemyGroupManager::resetGroups() {
	for (int i = 0; i < MAX_GROUPS; i++) {
		groups[i].numEnemies = 0;
		groups[i].triggeredYet = false;
		groups[i].active = false;
		groups[i].fadingIn = false;
		groups[i].fadingOut = false;
		groups[i].blockAlpha = 0.0;
	}
}

/**
 * Adds an enemy to an enemy group
 */
void EnemyGroupManager::addEnemy(int whichGroup) {
	groups[whichGroup].active = true;
	groups[whichGroup].numEnemies++;
}

/**
 * Notifies a group that an enemy died. When all enemiese in a group die
 */ 
void EnemyGroupManager::notifyOfDeath(int whichGroup) {
	
	//Ignore if whichGroup isn't active
	if (whichGroup < 0 || whichGroup >= MAX_GROUPS || !groups[whichGroup].active) return;

	//Decrement number of enemies left in the group
	groups[whichGroup].numEnemies--;

	//If the entire group is killed, start fading out the blocks
	if (groups[whichGroup].numEnemies <= 0) {
		groups[whichGroup].fadingOut = true;
		groups[whichGroup].fadingIn = false; //Prevents the block from both fading in and fading out at the same time, which would result in a permanent translucent block
	}

}

/**
 * Called every frame. Determines if the player has stepped on a trigger
 * square
 */
void EnemyGroupManager::update(float dt) {
	
	for (int i = 0; i < MAX_GROUPS; i++) {

		//Update fading in blocks
		if (groups[i].fadingIn) {
			groups[i].blockAlpha += 255.0*dt;
			if (groups[i].blockAlpha > 255.0) {
				groups[i].blockAlpha = 255.0;
				groups[i].fadingIn = false;
			}
		}
		
		//Update fading out blocks
		if (groups[i].fadingOut) {
			groups[i].blockAlpha -= 255.0*dt;
			if (groups[i].blockAlpha < 0.0) {
				groups[i].blockAlpha = 0.0;
				groups[i].fadingOut = false;
				groups[i].active = false;
				disableBlocks(i);
			}
		}

		//Check for a group that is triggered and not fading out but
		//doesn't have any enemies. This way the player won't get stuck
		//if they kill all the enemies before the blocks fade in
		if (groups[i].active && groups[i].triggeredYet && !groups[i].fadingOut && groups[i].numEnemies == 0) {
			groups[i].fadingIn = false;
			groups[i].fadingOut = true;;
		}

	}


	//If the player steps on a trigger
	if (smh->environment->ids[smh->player->gridX][smh->player->gridY] == ENEMYGROUP_TRIGGER) {
		triggerGroup(smh->environment->variable[smh->player->gridX][smh->player->gridY]);
	}

}

/**
 * Triggers an enemy group. Spawns enemy blocks and enemies associated
 * with that group.
 */
void EnemyGroupManager::triggerGroup(int whichGroup) {

	if (!groups[whichGroup].triggeredYet) {

		groups[whichGroup].triggeredYet = true;

		//Spawn enemies
		for (int i = 0; i < smh->environment->areaWidth; i++) {
			for (int j = 0; j < smh->environment->areaHeight; j++) {
				if (smh->environment->enemyLayer[i][j] != -1 &&
					smh->environment->ids[i][j] == ENEMYGROUP_ENEMY_POPUP &&
					smh->environment->variable[i][j] == whichGroup) {
						smh->enemyManager->addEnemy(smh->environment->enemyLayer[i][j], i, j, 0.25, 0.25, whichGroup);
						addEnemy(smh->environment->variable[i][j]);
						smh->environment->addParticle("treeletSpawn", i*64+32, j*64+32);
					}
			}
		}
		
		//Spawn enemy blocks if the player hasn't already killed all the enemies
		if (groups[whichGroup].numEnemies > 0) {
			enableBlocks(whichGroup);
		}
	}

}

/**
 * Enables all blocks for an enemy group.
 */
void EnemyGroupManager::enableBlocks(int whichGroup) {

	groups[whichGroup].fadingIn = true;

	for (int i = 0; i < smh->environment->areaWidth; i++) {
		for (int j = 0; j < smh->environment->areaHeight; j++) {

			//If this square is an enemy block for the triggered group
			if (smh->environment->ids[i][j] == ENEMYGROUP_BLOCK && 
					smh->environment->variable[i][j] == whichGroup) {

				//Set stuff in the environment to make an enemy block
				smh->environment->item[i][j] = ENEMYGROUP_BLOCKGRAPHIC;
				smh->environment->collision[i][j] = UNWALKABLE;
				smh->environment->addParticle("enemyBlockCloud", i*64.0+32.0, j*64.0+32.0);
			}
		}
	}
}

/**
 * Disables all blocks for an enemy group.
 */
void EnemyGroupManager::disableBlocks(int whichGroup) {
	for (int i = 0; i < smh->environment->areaWidth; i++) {
		for (int j = 0; j < smh->environment->areaHeight; j++) {
			if (smh->environment->ids[i][j] == ENEMYGROUP_BLOCK && 
					smh->environment->variable[i][j] == whichGroup) {
				smh->environment->item[i][j] = 0;
				smh->environment->collision[i][j] = WALKABLE;
			}
		}
	}
}
