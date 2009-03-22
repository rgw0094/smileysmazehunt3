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
E_DiagoShooter::E_DiagoShooter(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	//Start in wander state
	setState(new ES_Wander(this));

	facing = DOWN;
}

/**
 * Destructor
 */
E_DiagoShooter::~E_DiagoShooter() {

}


/**
 * Draws the enemy
 */
void E_DiagoShooter::draw(float dt) {
	graphic[facing]->Update(dt);
	graphic[facing]->Render(screenX, screenY);

	if (smh->isDebugOn()) {
		smh->drawCollisionBox(collisionBox, RED);
	}
}

/**
 * Updates the enemy
 */
void E_DiagoShooter::update(float dt) {

	int xDist = x - smh->player->x;
	int yDist = y - smh->player->y;
	int distanceDifference = abs(xDist) - abs(yDist); //when this is 0, the enemy is diagonal to Smiley

	if (smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay &&
		canShootPlayer() &&	abs(distanceDifference) <= smh->player->radius/2) { //within range and approximately diagonal
			
		float projectileAngle = 0;
	
		if (xDist > 0 && yDist > 0) projectileAngle = 3*3.14159/4;
		if (xDist > 0 && yDist < 0) projectileAngle = 5*3.14159/4;
		if (xDist < 0 && yDist > 0) projectileAngle = 1*3.14159/4;
		if (xDist < 0 && yDist < 0) projectileAngle = 7*3.14159/4;

		smh->projectileManager->addProjectile(x, y, projectileSpeed, projectileAngle, projectileDamage,
			true, projectileHoming, rangedType, true);
	} else { // move to be diagonal to smiley
		
	}
		
	//Collision with player
	if (smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "E_DiagoShooter.cpp Smiley hit by enemy type " + Util::intToString(id) +
			" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
		smh->setDebugText(debugText);
	}

	move(dt);
}
