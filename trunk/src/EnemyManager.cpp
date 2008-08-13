#include "environment.h"
#include "EnemyManager.h"
#include "enemy.h"
#include "lootmanager.h"
#include "ProjectileManager.h"
#include "player.h"
#include "EnemyGroupManager.h"
#include "GameData.h"
#include "CollisionCircle.h"
#include "SaveManager.h"

#include "hgerect.h"
#include "hgeparticle.h"
#include "hgeresource.h"

extern HGE *hge;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern LootManager *lootManager;
extern ProjectileManager *projectileManager;
extern bool debugMode;
extern hgeResourceManager *resources;
extern EnemyGroupManager *enemyGroupManager;
extern GameData *gameData;
extern SaveManager *saveManager;
extern float gameTime;

/**
 * Constructor
 */
EnemyManager::EnemyManager() { 
	deathParticles = new hgeParticleManager();
}

/**
 * Destructor
 */
EnemyManager::~EnemyManager() {
	reset();
	if (deathParticles) delete deathParticles;
}

bool SortEnemiesPredicate(const EnemyStruct &lhs, const EnemyStruct &rhs) {
  return lhs.enemy->y < rhs.enemy->y;
}


/**
 * Add an enemy to the list
 */
void EnemyManager::addEnemy(int id, int x, int y, float spawnHealthChance, float spawnManaChance, int groupID) {

	EnemyStruct newEnemy;
	newEnemy.spawnHealthChance = spawnHealthChance;
	newEnemy.spawnManaChance = spawnManaChance;

	switch (gameData->getEnemyInfo(id).enemyType) {
		case ENEMY_EVIL_EYE:
			newEnemy.enemy = new E_EvilEye(id, x, y, groupID);
			newEnemy.spawnHealthChance = 0.0;
			newEnemy.spawnManaChance = 0.0;
			break;
		case ENEMY_GUMDROP:
			newEnemy.enemy = new E_Gumdrop(id, x, y, groupID);
			break;
		case ENEMY_BOMB_GENERATOR:
			newEnemy.enemy = new E_BombGenerator(id, x, y, groupID);
			break;
		case ENEMY_CHARGER:
			newEnemy.enemy = new E_Charger(id, x, y, groupID);
			break;
		case ENEMY_CLOWNCRAB:
			newEnemy.enemy = new E_ChainClown(id, x, y, groupID);
			break;
		case ENEMY_BATLET_DIST:
			newEnemy.enemy = new E_BatletDist(id, x, y, groupID);
			break;
		case ENEMY_BUZZARD:
			newEnemy.enemy = new E_Buzzard(id, x, y, groupID);
			break;
		case ENEMY_SAD_SHOOTER:
			newEnemy.enemy = new E_SadShooter(id, x, y, groupID);
			break;
		case ENEMY_FLOATER:
			newEnemy.enemy = new E_Floater(id, x, y, groupID);
			break;
		case ENEMY_FLAILER:
			newEnemy.enemy = new E_Flailer(id, x, y, groupID);
			break;
		case ENEMY_TENTACLE:
			newEnemy.enemy = new E_Tentacle(id, x, y, groupID);
			break;
		case ENEMY_TURRET:
			newEnemy.enemy = new E_Turret(id, x, y, groupID);
			break;
		case ENEMY_GHOST:
			newEnemy.enemy = new E_Ghost(id, x, y, groupID);
			break;
		case ENEMY_FAKE:
			newEnemy.enemy = new E_Fake(id, x, y, groupID);
			break;
		case ENEMY_RANGED:
			newEnemy.enemy = new E_Ranged(id, x, y, groupID);
			break;
		case ENEMY_HOPPER:
			newEnemy.enemy = new E_Hopper(id, x, y, groupID);
			break;
		default:
			newEnemy.enemy = new DefaultEnemy(id, x, y, groupID);
			break;
	}
	
	enemyList.push_back(newEnemy);

}

void EnemyManager::killEnemies(int type) {
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		if (i->enemy->id == type) {
			//Notify enemy group
			enemyGroupManager->notifyOfDeath(i->enemy->groupID);

			//Play death sound effect
			if (i->enemy->frozen) {
				hge->Effect_Play(resources->GetEffect("snd_iceDie"));
			}

			//Death effects
			deathParticles->SpawnPS(&resources->GetParticleSystem("deathCloud")->info, i->enemy->x, i->enemy->y);
			hge->Effect_Play(resources->GetEffect("snd_enemyDeath"));

			//Spawn loot
			randomLoot = hge->Random_Int(0,10000);
			if (randomLoot < 10000.0 * i->spawnHealthChance) {
				lootManager->addLoot(LOOT_HEALTH, i->enemy->x, i->enemy->y, -1);
			} else if (randomLoot < 10000.0 * i->spawnHealthChance + 10000.0*i->spawnManaChance) {
				lootManager->addLoot(LOOT_MANA, i->enemy->x, i->enemy->y, -1);
			}

			delete i->enemy;
			i = enemyList.erase(i);
		}
	}
}

/**
 * Draw all the enemies in the list
 */
void EnemyManager::draw(float dt) {

	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		i->enemy->baseDraw(dt);
	}

	//Draw death effect
	deathParticles->Update(dt);
	deathParticles->Transpose(-1*(theEnvironment->xGridOffset*64 + theEnvironment->xOffset), -1*(theEnvironment->yGridOffset*64 + theEnvironment->yOffset));
	deathParticles->Render();

}

/**
 * Update all the enemies in the list
 */
void EnemyManager::update(float dt) {

	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {

		//Call the base update method for the enemy.
		i->enemy->baseUpdate(dt);

		//If the enemy is dead
		if (i->enemy->health <= 0.0f) {

			//Notify enemy group
			enemyGroupManager->notifyOfDeath(i->enemy->groupID);

			//Play death sound effect
			if (i->enemy->frozen) {
				hge->Effect_Play(resources->GetEffect("snd_iceDie"));
			}

			//Death effects
			deathParticles->SpawnPS(&resources->GetParticleSystem("deathCloud")->info, i->enemy->x, i->enemy->y);
			hge->Effect_Play(resources->GetEffect("snd_enemyDeath"));

			//Spawn loot
			randomLoot = hge->Random_Int(0,10000);
			if (randomLoot < 10000.0 * i->spawnHealthChance) {
				lootManager->addLoot(LOOT_HEALTH, i->enemy->x, i->enemy->y, -1);
			} else if (randomLoot < 10000.0 * i->spawnHealthChance + 10000.0*i->spawnManaChance) {
				lootManager->addLoot(LOOT_MANA, i->enemy->x, i->enemy->y, -1);
			}

			delete i->enemy;
			i = enemyList.erase(i);

			saveManager->numEnemiesKilled++;

		}
	}

	enemyList.sort(SortEnemiesPredicate);

}

/**
 * Returns whether or not the provided collision box collides with
 * any enemies
 */
bool EnemyManager::testCollision(hgeRect *collisionBox) {
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		if (collisionBox->Intersect(i->enemy->collisionBox)) {
			return true;
		}
	}
}

/**
 * Returns true if the collision circle collides with a frozen enemy
 */
bool EnemyManager::collidesWithFrozenEnemy(CollisionCircle *circle) {
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		//Check collision
		if (i->enemy->frozen && circle->testBox(i->enemy->collisionBox)) {
			return true;
		}
	}
}

/**
 * Damages and knocks back any enemy hit by Smiley's tongue. Returns whether
 * or not an enemy was hit.
 *
 *	weaponBox			the collision box of the weapon
 *	damage				how much damage the weapon does if it hits
 */
bool EnemyManager::tongueCollision(Tongue *tongue, float damage) {
	bool hit = false;
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		if (!i->enemy->immuneToTongue) {
			if (i->enemy->doTongueCollision(tongue, damage)) {
				i->enemy->notifyTongueHit();
				hit = true;
			}
		}
	}
	return hit;
} //end tongueCollision()


/**
 * Freezes any enemies within the box argument
 */
void EnemyManager::freezeEnemies(int x, int y) {
	bool dickens = false;
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		//Check collision
		if (!gameData->getEnemyInfo(i->enemy->id).immuneToFreeze && i->enemy->collisionBox->TestPoint(x,y)) {
			//Freeze sound effect is fucked up
			//if (!dickens) hge->Effect_Play(resources->GetEffect("snd_freeze"));
			dickens = true;
			i->enemy->frozen = true;
			i->enemy->timeFrozen = hge->Timer_GetTime();
		}
	}
}


/**
 * Unfreezes any frozen enemies within the box argument
 */
void EnemyManager::unFreezeEnemies(int x, int y) {
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		//Check collision
		if (i->enemy->collisionBox->TestPoint(x,y)) i->enemy->frozen = false;
	}
}

/**
 * If any enemies collide with the projectile's collision box, deal
 * damage and apply any special effects from the projectile.
 *
 * Returns whether or not any enemies were hit
 */
bool EnemyManager::hitEnemiesWithProjectile(hgeRect *collisionBox, float damage, int type) {
		
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		//Check collision
		if (i->enemy->collisionBox->Intersect(collisionBox)) {

			//Notify the enemy of what type of projectile it was hit with
			i->enemy->hitWithProjectile(type);

			if (gameData->getEnemyInfo(i->enemy->id).invincible) return true;

			if (type == PROJECTILE_FRISBEE) {
				if (!i->enemy->immuneToStun) {
					i->enemy->stunned = true;
					i->enemy->stunLength = 2.0f;
					i->enemy->startedStun = gameTime;
				}			
			} 

			if (!(type == PROJECTILE_LIGHTNING_ORB && i->enemy->immuneToLightning)) {
				i->enemy->dealDamageAndKnockback(damage, 0.0, 0.0, 0.0);
				i->enemy->startFlashing();
			}

			return true;
		}
	}
	return false;
}


/**
 * Deletes all the managed enemies
 */
void EnemyManager::reset() {
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		delete i->enemy;
		i = enemyList.erase(i);
	}
	enemyList.clear();
}







