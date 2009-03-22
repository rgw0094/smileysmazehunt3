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
E_Botonoid::E_Botonoid(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	//Start in wander state
	setState(new ES_Wander(this));

	facing = DOWN;
}

/**
 * Destructor
 */
E_Botonoid::~E_Botonoid() {

}


/**
 * Draws the enemy
 */
void E_Botonoid::draw(float dt) {
	graphic[facing]->Update(dt);
	graphic[facing]->Render(screenX, screenY);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, RED);
	}
}

/**
 * Updates the enemy
 */
void E_Botonoid::update(float dt) {
	

	if (smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay &&
		Util::distance(x,y,smh->player->x,smh->player->y) <=  weaponRange) {
			if (smh->randomFloat(0,1.00) <= dt) {
				//fire a projectile
				if (hasRangedAttack) {
					smh->projectileManager->addProjectile(x, y, projectileSpeed, smh->randomFloat(0,2*3.14159), projectileDamage,
						true,projectileHoming, rangedType, true);
					lastRangedAttack = smh->getGameTime();
				}
			}
	}

	
		
	//Collision with player
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "E_Botonoid.cpp Smiley hit by enemy type " + Util::intToString(id) +
			" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
		smh->setDebugText(debugText);
	}

	move(dt);
}
