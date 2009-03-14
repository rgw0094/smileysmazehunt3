#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "CollisionCircle.h"

extern SMH *smh;

/**
 * Constructor
 */
E_AdjacentShooter::E_AdjacentShooter(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	//Start in wander state
	setState(new ES_Wander(this));

	facing = DOWN;
}

/**
 * Destructor
 */
E_AdjacentShooter::~E_AdjacentShooter() {

}


/**
 * Draws the enemy
 */
void E_AdjacentShooter::draw(float dt) {
	graphic[facing]->Update(dt);
	graphic[facing]->Render(screenX, screenY);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, RED);
	}
}

/**
 * Updates the enemy
 */
void E_AdjacentShooter::update(float dt) {
	

	if (smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay &&
		Util::distance(x,y,smh->player->x,smh->player->y) <=  weaponRange) {
	
		//if smiley is in same x grid
		if (gridX == smh->player->gridX) {
			if (smh->player->y < y) { //player is above enemy; shoot up
				smh->projectileManager->addProjectile(x, y, projectileSpeed, 3*3.14159/2, projectileDamage,
					true,projectileHoming, rangedType, true);
				lastRangedAttack = smh->getGameTime();
			} else { //player is below enemy; shoot down
				smh->projectileManager->addProjectile(x, y, projectileSpeed, 3.14159/2, projectileDamage,
					true,projectileHoming, rangedType, true);	
				lastRangedAttack = smh->getGameTime();
			}
		} else if (gridY == smh->player->gridY) {
			if (smh->player->x < x) { //player is to the left
				smh->projectileManager->addProjectile(x, y, projectileSpeed, 3.14159, projectileDamage,
					true,projectileHoming, rangedType, true);
				lastRangedAttack = smh->getGameTime();
			} else { //player is to the right
				smh->projectileManager->addProjectile(x, y, projectileSpeed, 0, projectileDamage,
					true,projectileHoming, rangedType, true);	
				lastRangedAttack = smh->getGameTime();
			}
		}
		
	}

	
		
	//Collision with player
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "E_AdjacentShooter.cpp Smiley hit by enemy type " + Util::intToString(id) +
			" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
		smh->setDebugText(debugText);
	}

	move(dt);
}
