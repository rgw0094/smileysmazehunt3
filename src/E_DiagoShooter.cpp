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

	hasDestination = false;
	timeOfLastDestination = smh->getGameTime();
	timeIntervalTillNextDestination = 0;
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
	
		if (xDist > 0 && yDist > 0) projectileAngle = 5*3.14159/4.0;
		if (xDist > 0 && yDist < 0) projectileAngle = 3*3.14159/4.0;
		if (xDist < 0 && yDist > 0) projectileAngle = 7*3.14159/4.0;
		if (xDist < 0 && yDist < 0) projectileAngle = 1*3.14159/4.0;

		smh->projectileManager->addProjectile(x, y, projectileSpeed, projectileAngle, projectileDamage,
			true, projectileHoming, rangedType, true);
	} else { // move to be diagonal to smiley
		moveToBeDiagonalToPlayer();
		move(dt);
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

}

/**
 * This method moves the enemy to try to be diagonal to Smiley, so that the enemy can get a clear shot.
 */
void E_DiagoShooter::moveToBeDiagonalToPlayer() {
	
	//creates a new destinaton
	if (smh->timePassedSince(timeOfLastDestination) >= timeIntervalTillNextDestination) createDestinationToBeDiagonalToPlayer();
	
	//moves toward a destination -- this is shamelessly copied from ES_Chase.cpp
	if (hasDestination) {
		int lowValue = mapPath[gridX][gridY];
			
		//Find the best square to go to next
		for (int i = gridX - 1; i <= gridX+1; i++) {
			for (int j = gridY-1; j <= gridY+1; j++) {
				if (smh->environment->isInBounds(i,j) && mapPath[i][j] >= 0 && mapPath[i][j] < 999) {
					if (mapPath[i][j] <= lowValue) {
						lowValue = mapPath[i][j];
						targetX = i;
						targetY = j;
					}
				}
			}
		}
		
		//Update velocity based on best path
		if (targetX < gridX) dx = -1*speed;
		if (targetX > gridX) dx = speed;
		if (targetY < gridY) dy = -1*speed;
		if (targetY > gridY) dy = speed;

		setFacing();
		
	} else {
		dx = 0;
		dy = 0;
		facing = DOWN;
	}


}

/**
 * This method creates the destination for the "moveToBeDiagonalToSmiley" method to work 
 *
 *-Using the A* algorithm:
 *-The enemy first searches to see if it can get at diagonal grid spots which are almost at the max weapon range
 *-If none of those work, the enemy tries 1 block closer, and so on.
 *-If the enemy cannot get to a location, then it just holds still.
 */

void E_DiagoShooter::createDestinationToBeDiagonalToPlayer() {

	int xDist = x - smh->player->x;
	int yDist = y - smh->player->y;
	
	float oneLegWeaponRange = weaponRange * 1/sqrt(2.0); //the "leg" of a triangle of 45 degrees is 1 / root 2
	
	oneLegWeaponRange *= 0.8; //don't want to be so close to the edge of the weapon range, so take away 20%
	
	int weaponGridRange = oneLegWeaponRange / 64;

	hasDestination = false;

    // OK now let's see which quadrant the Cone is in relative to Smiley, and test to see if
	// we can get to anywhere along that diagonal
	
	/*              |
	 *      x<0,y<0 |  x>0,y<0
	 *         II   |   I
	 *      -------------------
	 *        III   |   IV
	 *      x<0,y>0 |  x>0,y>0
	 *              |
	 */
	
	int d;

	// Quadrant I, up and right of Smiley ////////////////////
	if (xDist >= 0 && yDist < 0) { 
		if (abs(xDist) > abs(yDist)) { //try up-right, then down-right, then up-left, then down-left
			hasDestination = tryUpRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownLeftDiagonal(weaponGridRange);
		} else { //try up-right, then up-left, then down-right, then down-left
			hasDestination = tryUpRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownLeftDiagonal(weaponGridRange);
		}

	// Quadrant II, up and left of Smiley ////////////////////
	} else if (xDist < 0 && yDist < 0) {
		if (abs(xDist) > abs(yDist)) { //try up-left, down-left, up-right, down-right
			hasDestination = tryUpLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownRightDiagonal(weaponGridRange);
		} else { //try up-left, up-right, down-left, down-right
			hasDestination = tryUpLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownRightDiagonal(weaponGridRange);
		}

		// Quadrant III, down and left of Smiley ////////////////////
    	} else if (xDist < 0 && yDist >= 0) {
		if (abs(xDist) > abs(yDist)) { //try down-left, up-left, down-right, up-right
			hasDestination = tryDownLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpRightDiagonal(weaponGridRange);
		} else { //try down-left, down-right, up-left, up-right
			hasDestination = tryDownLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpRightDiagonal(weaponGridRange);
		}

	// Quadrant IV, down and right of Smiley ////////////////////
	} else if (xDist >= 0 && yDist >= 0) {
		if (abs(xDist) > abs(yDist)) { //try down-right, up-right, down-left, up-left
			hasDestination = tryDownRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpLeftDiagonal(weaponGridRange);
		} else { //try down-right, down-left, up-right, up-left
			hasDestination = tryDownRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryDownLeftDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpRightDiagonal(weaponGridRange);
			if (!hasDestination) hasDestination = tryUpLeftDiagonal(weaponGridRange);			
		}
	}

	timeOfLastDestination = smh->getGameTime();
	timeIntervalTillNextDestination = smh->randomFloat(1.0,1.7);
}


/* tryUpRightDiagonal *********
 * Calls tryDiagonal, to create an A* path from the enemy to a position up and right of Smiley
 */
bool E_DiagoShooter::tryUpRightDiagonal(int weaponGridRange) {
	return tryDiagonal(weaponGridRange, 1, -1);
}

/* tryUpLeftDiagonal *********
 * Calls tryDiagonal, to create an A* path from the enemy to a position up and left of Smiley
 */
bool E_DiagoShooter::tryUpLeftDiagonal(int weaponGridRange) {
	return tryDiagonal(weaponGridRange, -1, -1);
}

/* tryDownRightDiagonal *********
 * Calls tryDiagonal, to create an A* path from the enemy to a position down and right of Smiley
 */
bool E_DiagoShooter::tryDownRightDiagonal(int weaponGridRange) {
	return tryDiagonal(weaponGridRange, 1, 1);
}

/* tryDownLeftDiagonal *********
 * Calls tryDiagonal, to create an A* path from the enemy to a position down and left of Smiley
 */
bool E_DiagoShooter::tryDownLeftDiagonal(int weaponGridRange) {
	return tryDiagonal(weaponGridRange, -1, 1);
}

/* tryDiagonal ***********
 * 
 * This function loops from far to near, creating an A* algorithm at each point and seeing if the enemy can
 * get to that point
 *
 * dirx and diry give directionality to which way the destinationGrid points are
 */
bool E_DiagoShooter::tryDiagonal(int weaponGridRange, int dirx, int diry) {
	int d;

	//First see the max that a shot could potentially reach Smiley at from that location
	for (d = 1; d < weaponGridRange; d++) {
		if (!smh->projectileManager->canPass[smh->environment->collision[smh->player->gridX][smh->player->gridY]]) {
			weaponGridRange = d - 1;
			break;
		}
	}

	//Now loop from out to in, creating A* algorithms
	for (d = weaponGridRange; d > 1; d--) {
		destinationGridX = gridX + d*dirx;
		destinationGridY = gridY + d*diry;
			
		doAStar(destinationGridX, destinationGridY);
		if (markMap && mapPath[gridX][gridY] < 999) { //enemy could get to this point; let's go there!
			return true;			
		} 
	}
	return false;
}
