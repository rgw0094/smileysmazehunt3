#include "SmileyEngine.h"
#include "environment.h"
#include "EnemyFramework.h"
#include "lootmanager.h"
#include "ProjectileManager.h"
#include "player.h"
#include "CollisionCircle.h"

#include "hgerect.h"
#include "hgeparticle.h"
#include "hgeresource.h"

extern SMH *smh;

/**
 * Constructor
 */
EnemyManager::EnemyManager() { 
	deathParticles = new hgeParticleManager();
	toDrawImmunities=false;
}

/**
 * Destructor
 */
EnemyManager::~EnemyManager() {
	//Never deleted
}

bool SortEnemiesPredicate(const EnemyStruct &lhs, const EnemyStruct &rhs) {
  return lhs.enemy->y < rhs.enemy->y;
}


/**
 * Add an enemy to the list
 */
void EnemyManager::addEnemy(int id, int gridX, int gridY, float spawnHealthChance, float spawnManaChance, int groupID, bool useSpawningEffect) {

	EnemyStruct newEnemy;

	newEnemy.spawnHealthChance = spawnHealthChance;
	newEnemy.spawnManaChance = spawnManaChance;

	switch (smh->gameData->getEnemyInfo(id).enemyType) {
		
		case ENEMY_EVIL_EYE:
			newEnemy.enemy = new E_EvilEye(id, gridX, gridY, groupID);
			newEnemy.spawnHealthChance = 0.0;
			newEnemy.spawnManaChance = 0.0;
			break;
		case ENEMY_GUMDROP:
			newEnemy.enemy = new E_Gumdrop(id, gridX, gridY, groupID);
			break;
		case ENEMY_BOMB_GENERATOR:
			newEnemy.enemy = new E_BombGenerator(id, gridX, gridY, groupID);
			break;
		case ENEMY_CHARGER:
			newEnemy.enemy = new E_Charger(id, gridX, gridY, groupID);
			break;
		case ENEMY_CLOWNCRAB:
			newEnemy.enemy = new E_ChainClown(id, gridX, gridY, groupID);
			break;
		case ENEMY_BATLET_DIST:
			newEnemy.enemy = new E_BatletDist(id, gridX, gridY, groupID);
			break;
		case ENEMY_BUZZARD:
			newEnemy.enemy = new E_Buzzard(id, gridX, gridY, groupID);
			break;
		case ENEMY_SAD_SHOOTER:
			newEnemy.enemy = new E_SadShooter(id, gridX, gridY, groupID);
			break;
		case ENEMY_FLOATER:
			newEnemy.enemy = new E_Floater(id, gridX, gridY, groupID);
			break;
		case ENEMY_FLAILER:
			newEnemy.enemy = new E_Flailer(id, gridX, gridY, groupID);
			break;
		case ENEMY_TENTACLE:
			newEnemy.enemy = new E_Tentacle(id, gridX, gridY, groupID);
			break;
		case ENEMY_TURRET:
			newEnemy.enemy = new E_Turret(id, gridX, gridY, groupID);
			break;
		case ENEMY_GHOST:
			newEnemy.enemy = new E_Ghost(id, gridX, gridY, groupID);
			break;
		case ENEMY_FAKE:
			newEnemy.enemy = new E_Fake(id, gridX, gridY, groupID);
			break;
		case ENEMY_RANGED:
			newEnemy.enemy = new E_Ranged(id, gridX, gridY, groupID);
			break;
		case ENEMY_HOPPER:
			newEnemy.enemy = new E_Hopper(id, gridX, gridY, groupID);
			break;
		case ENEMY_SPAWNER:
			newEnemy.enemy = new E_Spawner(id, gridX, gridY, groupID);
			break;
		case ENEMY_ADJACENT_SHOOTER:
			newEnemy.enemy = new E_AdjacentShooter(id, gridX, gridY, groupID);
			break;
		case ENEMY_BOTONOID:
			newEnemy.enemy = new E_Botonoid(id, gridX, gridY, groupID);
			break;
		case ENEMY_DIAGO_SHOOTER:
			newEnemy.enemy = new E_DiagoShooter(id, gridX, gridY, groupID);
			break;
		case ENEMY_FENWAR_EYE_SPIDER:
			newEnemy.enemy = new E_FenwarEyeSpider(id, gridX, gridY, groupID);
			break;
		default:
			newEnemy.enemy = new DefaultEnemy(id, gridX, gridY, groupID);
			break;
	}

	if (useSpawningEffect) {
		newEnemy.enemy->isSpawning = true;
		newEnemy.enemy->spawnState = ENEMY_SPAWNSTATE_FALLING;
		newEnemy.enemy->spawnSize = 0.4;	
		newEnemy.enemy->spawnY = 45.0; //how high above the ground the enemy spawns
		newEnemy.enemy->beginSpawnTime = smh->getGameTime();
	} else {
		newEnemy.enemy->isSpawning = false;
	}
	
	enemyList.push_back(newEnemy);

}

/**
 * Kills all enemies of the specified type.
 */
void EnemyManager::killEnemies(int type) {
	for (std::list<EnemyStruct>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		if (i->enemy->id == type) {
			killEnemy(i);
			delete i->enemy;
			i = enemyList.erase(i);
		}
	}
}

/**
 * Kills all enemies of the given type in the given box.
 */ 
void EnemyManager::killEnemiesInBox(hgeRect *box, int type) {
	for (std::list<EnemyStruct>::iterator i = enemyList.begin(); i != enemyList.end(); i++) {
		if (i->enemy->collisionBox->Intersect(box) && i->enemy->id == type) {
			killEnemy(i);
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

	if (toDrawImmunities) {
		
		for (i = enemyList.begin(); i != enemyList.end(); i++) {
			i->enemy->drawImmunities();
		}
		toDrawImmunities=false;
	}

	//Draw death effect
	deathParticles->Update(dt);
	deathParticles->Transpose(-1*(smh->environment->xGridOffset*64 + smh->environment->xOffset), -1*(smh->environment->yGridOffset*64 + smh->environment->yOffset));
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
			killEnemy(i);
			delete i->enemy;
			i = enemyList.erase(i);
		}
	}

	//Sort enemies so that the rearmost overlapping enemies will be drawn last!!!
	enemyList.sort(SortEnemiesPredicate);

}

void EnemyManager::spawnDeathParticle(float x, float y)
{
	deathParticles->SpawnPS(&smh->resources->GetParticleSystem("deathCloud")->info, x, y);
}

void EnemyManager::killEnemy(std::list<EnemyStruct>::iterator i) 
{
	smh->enemyGroupManager->notifyOfDeath(i->enemy->groupID);
	i->enemy->notifyOfDeath();

	std::string debugText;
	debugText = "Enemy type " + Util::intToString(i->enemy->id) + " died at " + Util::intToString(Util::getGridX(i->enemy->x)) + "," + Util::intToString(Util::getGridY(i->enemy->y));
	smh->setDebugText(debugText);

	if (i->enemy->frozen) 
	{
		smh->soundManager->playSound("snd_iceDie");
	} 
	else 
	{
		spawnDeathParticle(i->enemy->x, i->enemy->y);
		smh->soundManager->playSound("snd_enemyDeath");
	}

	//Spawn loot
	randomLoot = smh->randomInt(0,10000);
	if (randomLoot < 10000.0 * i->spawnHealthChance) 
	{
		smh->lootManager->addLoot(LOOT_HEALTH, i->enemy->x, i->enemy->y, -1);
	} 
	else if (randomLoot < 10000.0 * i->spawnHealthChance + 10000.0*i->spawnManaChance) 
	{
		smh->lootManager->addLoot(LOOT_MANA, i->enemy->x, i->enemy->y, -1);
	}

	smh->saveManager->numEnemiesKilled++;
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
	return false;
}

/**
 * Returns whether or not the provided collision box collides with
 * any enemies, excluding turrets.
 */
bool EnemyManager::testCollisionExcludingTurrets(hgeRect *collisionBox) {
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		if ((i->enemy->enemyType >= 31 && i->enemy->enemyType <= 31+12) || (i->enemy->enemyType >= 52 && i->enemy->enemyType <= 52+12)) {
			//is a turret
		} else { //not a turret
			if (collisionBox->Intersect(i->enemy->collisionBox)) {
				return true;
			}
		}
	}
	return false;
}

/**
 * Returns true if there is an enemy of a certain type in collision with the collisionRect
 *
 * Exclude a specific x and y so that the enemy does not recognize himself.
 */
bool EnemyManager::testCollisionCertainEnemies(hgeRect *collisionBox, int enemyIDToTest, float specificEnemyXToExclude, float specificEnemyYToExclude) {
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		if (i->enemy->enemyType == enemyIDToTest && i->enemy->x != specificEnemyXToExclude && i->enemy->y != specificEnemyYToExclude) {
			//is the correct enemy type
			if (collisionBox->Intersect(i->enemy->collisionBox)) {
				return true;
			}
		}
			
	}
	return false;
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
	return false;
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
		} else {
			if (smh->player->getTongue()->testCollision(i->enemy->collisionBox)) {
				smh->soundManager->playSound("snd_HitInvulnerable");
			}
		}
	}

	if (hit) {
		playHitSoundEffect();
	}

	return hit;
} //end tongueCollision()


/**
 * Freezes any enemies within the box argument
 */
void EnemyManager::freezeEnemies(int x, int y) {
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {
		//Check collision
		if (i->enemy->collisionBox->TestPoint(x,y)) {
			if (!smh->gameData->getEnemyInfo(i->enemy->id).immuneToFreeze) { 
				i->enemy->frozen = true;
				i->enemy->timeFrozen = smh->getGameTime();
			} else {
				smh->soundManager->playSound("snd_HitInvulnerable");
			}
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
bool EnemyManager::hitEnemiesWithProjectile(hgeRect *collisionBox, float damage, int type, float stunPower) {
		
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) {

		//If the enemy is NOT a turret, and the bullet IS a turret bullet, then the projectile passes through the enemy			
		if (smh->gameData->getEnemyInfo(i->enemy->id).enemyType != ENEMY_TURRET && type == PROJECTILE_TURRET_CANNONBALL) {
				//do nothing, this way the turret's cannonball passes through the enemy of interest
		//Check collision
		} else if (i->enemy->collisionBox->Intersect(collisionBox)) {		

			//Notify the enemy of what type of projectile it was hit with
			i->enemy->hitWithProjectile(type);

			if (smh->gameData->getEnemyInfo(i->enemy->id).invincible) return true;

			if (stunPower > 0.0) {
				if (!i->enemy->immuneToStun) {
					i->enemy->stunned = true;
					i->enemy->stunLength = stunPower;
					i->enemy->startedStun = smh->getGameTime();
				} else {
					smh->soundManager->playSound("snd_HitInvulnerable");
				}
			} 

			if (!(type == PROJECTILE_LIGHTNING_ORB && i->enemy->immuneToLightning)) {
				i->enemy->dealDamageAndKnockback(damage, 0.0, 0.0, 0.0);
				if (damage > 0.0) i->enemy->startFlashing();
			} else if (type==PROJECTILE_LIGHTNING_ORB && i->enemy->immuneToLightning) {
				smh->soundManager->playSound("snd_HitInvulnerable");
			}

			return true;
		}
	}
	return false;
}

void EnemyManager::playHitSoundEffect() {
	switch (smh->hge->Random_Int(1,5)) {
		case 1:
			smh->soundManager->playSound("snd_Hit1");
			break;
		case 2:
			smh->soundManager->playSound("snd_Hit2");
			break;
		case 3:
			smh->soundManager->playSound("snd_Hit3");
			break;
		case 4:
			smh->soundManager->playSound("snd_Hit4");
			break;
		case 5:
			smh->soundManager->playSound("snd_Hit5");
			break;
	}
}

/**
 * Calls the drawImmunities function in each enemy
 */
void EnemyManager::drawEnemyImmunities() {
	toDrawImmunities=true;
	
}

/**
 * Deletes all the managed enemies
 */
void EnemyManager::reset() 
{
	std::list<EnemyStruct>::iterator i;
	for (i = enemyList.begin(); i != enemyList.end(); i++) 
	{
		int enemyType = i->enemy->enemyType;
		try
		{
			delete i->enemy;
			i = enemyList.erase(i);
		} 
		catch (System::Exception *ex)
		{
			smh->hge->System_Log("Exception caught deleting enemy type %d", enemyType);
			smh->hge->System_Log("%s", ex->ToString());
		}

	}
	enemyList.clear();
}







