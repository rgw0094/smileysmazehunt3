/**
 * Enemy manager class. This should really be called EnemyManager.
 */
#include "smiley.h"
#include "enemies.h"
#include "enemy.h"
#include "lootmanager.h"
#include "projectiles.h"
#include "player.h"
#include "EnemyGroupManager.h"

extern HGE *hge;
extern Environment *theEnvironment;
extern Player *thePlayer;
extern LootManager *lootManager;
extern ProjectileManager *projectileManager;
extern bool debugMode;
extern hgeResourceManager *resources;
extern EnemyGroupManager *enemyGroupManager;
extern EnemyInfo enemyInfo[99];
extern float gameTime;

#define ENEMY_FLASH_DURATION 0.5
#define ENEMY_FROZEN_DURATION 3.0

/**
 * Constructor
 */
Enemies::Enemies() { 
	deathParticles = new hgeParticleManager();
}

/**
 * Destructor
 */
Enemies::~Enemies() {
	reset();
	if (deathParticles) delete deathParticles;
}

/**
 * Add an enemy to the list
 */
void Enemies::addEnemy(int id, int x, int y, float spawnHealthChance, float spawnManaChance, int groupID) {

	EnemyStruct newEnemy;
	newEnemy.spawnHealthChance = spawnHealthChance;
	newEnemy.spawnManaChance = spawnManaChance;

	switch (enemyInfo[id].enemyType) {
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
			newEnemy.enemy = new E_ClownCrab(id, x, y, groupID);
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
		default:
			newEnemy.enemy = new MeleeEnemy(id, x, y, groupID);
			break;
	}
	
	theEnemies.push_back(newEnemy);

}

/**
 * Draw all the enemies in the list
 */
void Enemies::draw(float dt) {
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {

		//Call the enemy's draw function. If the enemy is currently flashing,
		//skip calling it some frames to create the flashing effect.
		if (!i->enemy->flashing || int(gameTime * 100) % 10 > 5) {
			i->enemy->draw(dt);
		}

		//Draw the ice block over the enemy if its frozen
		if (i->enemy->frozen) {
			i->enemy->drawFrozen(dt);
		}

		//Stunned enemy
		if (i->enemy->stunned) {
			i->enemy->drawStunned(dt);
		}

		//Debug mode stuff
		if (debugMode && i->enemy->dealsCollisionDamage) {
			drawCollisionBox(i->enemy->collisionBox, RED);
		}
	}

	//Draw death effect
	deathParticles->Update(dt);
	deathParticles->Transpose(-1*(theEnvironment->xGridOffset*64 + theEnvironment->xOffset), -1*(theEnvironment->yGridOffset*64 + theEnvironment->yOffset));
	deathParticles->Render();

}

/**
 * Update all the enemies in the list
 */
void Enemies::update(float dt) {

	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {

		//Update basic shit that every enemy needs
		i->enemy->collisionBox->SetRadius(i->enemy->x, i->enemy->y, i->enemy->radius);
		i->enemy->gridX = i->enemy->x / 64;
		i->enemy->gridY = i->enemy->y / 64;
		i->enemy->screenX = i->enemy->x - theEnvironment->xGridOffset*64 - theEnvironment->xOffset; 
		i->enemy->screenY = i->enemy->y - theEnvironment->yGridOffset*64 - theEnvironment->yOffset;

		//Update statuses
		if (i->enemy->stunned && timePassedSince(i->enemy->startedStun) > i->enemy->stunLength) {
			i->enemy->stunned = false;
		}
		if (i->enemy->flashing && timePassedSince(i->enemy->timeStartedFlashing) > ENEMY_FLASH_DURATION) {
			i->enemy->flashing = false;
		}
		if (i->enemy->knockback && timePassedSince(i->enemy->startedKnockback) > i->enemy->knockbackTime) {
			i->enemy->knockback = false;
		}
		if (i->enemy->frozen && timePassedSince(i->enemy->timeFrozen) > ENEMY_FROZEN_DURATION) {
			i->enemy->frozen = false;
		}

		//Update default animations
		if (!i->enemy->frozen && !i->enemy->stunned) {
			for (int n = 0; n < 4; n++) {
				i->enemy->graphic[n]->Update(dt);
			}
		}

		//Update the enemy's current state
		if (i->enemy->currentState) {
			i->enemy->currentState->update(dt);
		}

		//Call the enemy's update method
		i->enemy->update(dt);
		
		//Do player collision
		if (i->enemy->dealsCollisionDamage && thePlayer->collisionCircle->testBox(i->enemy->collisionBox)) {
			thePlayer->dealDamageAndKnockback(i->enemy->damage, true, 115, i->enemy->x, i->enemy->y);
		}

		//Fire breath collision
		if (!i->enemy->immuneToFire) {
			if (thePlayer->fireBreathParticle->testCollision(i->enemy->collisionBox)) {
				i->enemy->frozen = false;
				i->enemy->dealDamageAndKnockback(thePlayer->getFireBreathDamage()*dt, 
					500.0*dt,thePlayer->x, thePlayer->y);
			}
		}

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
			randomLoot = rand() % 10000;
			if (randomLoot < 10000.0 * i->spawnHealthChance) {
				lootManager->addLoot(LOOT_HEALTH, i->enemy->x, i->enemy->y, -1);
			} else if (randomLoot < 10000.0 * i->spawnHealthChance + 10000.0*i->spawnManaChance) {
				lootManager->addLoot(LOOT_MANA, i->enemy->x, i->enemy->y, -1);
			}

			delete i->enemy;
			i = theEnemies.erase(i);

		}
	}

}

/**
 * Returns whether or not the provided collision box collides with
 * any enemies
 */
bool Enemies::collidesWithEnemy(hgeRect *collisionBox) {
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
		if (collisionBox->Intersect(i->enemy->collisionBox)) {
			return true;
		}
	}
}

/**
 * Returns true if the collision circle collides with a frozen enemy
 */
bool Enemies::collidesWithFrozenEnemy(CollisionCircle *circle) {
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
		//Check collision
		if (i->enemy->frozen && circle->testBox(i->enemy->collisionBox)) {
			return true;
		}
	}
}



/**
 * Damages and knocks back any enemy hit by Smiley's tongue.
 *
 *	weaponBox			the collision box of the weapon
 *	damage				how much damage the weapon does if it hits
 */
void Enemies::tongueCollision(Tongue *tongue, float damage) {
	
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
		if (!i->enemy->immuneToTongue) {
			i->enemy->doTongueCollision(tongue, damage);
		}
	}

} //end tongueCollision()


/**
 * Freezes any enemies within the box argument
 */
void Enemies::freezeEnemies(int x, int y) {
	bool dickens = false;
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
		//Check collision
		if (!enemyInfo[i->enemy->id].immuneToFreeze && i->enemy->collisionBox->TestPoint(x,y)) {
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
void Enemies::unFreezeEnemies(int x, int y) {
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
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
bool Enemies::hitEnemiesWithProjectile(hgeRect *collisionBox, float damage, int type) {
	//Loop through the enemies
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
		//Check collision
		if (i->enemy->collisionBox->Intersect(collisionBox)) {

			i->enemy->hitWithProjectile(type);

			if (enemyInfo[i->enemy->id].invincible) return true;

			if (type == PROJECTILE_LIGHTNING_ORB) {
				if (!i->enemy->immuneToLightning) {
					i->enemy->dealDamageAndKnockback(damage, 0.0, 0.0, 0.0);
					i->enemy->startFlashing();
				}
			} else if (type == PROJECTILE_FRISBEE) {
				if (!i->enemy->immuneToStun) {
					i->enemy->stunned = true;
					i->enemy->stunLength = 2.0f;
					i->enemy->startedStun = hge->Timer_GetTime();
				}			
			} else {
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
void Enemies::reset() {
	std::list<EnemyStruct>::iterator i;
	for (i = theEnemies.begin(); i != theEnemies.end(); i++) {
		delete i->enemy;
		i = theEnemies.erase(i);
	}
}







