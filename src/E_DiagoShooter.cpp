#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "hgeresource.h"
#include "ProjectileManager.h"
#include "player.h"
#include "environment.h"
#include "CollisionCircle.h"

extern SMH *smh;

#define TARGET_ERROR 3.0

#define DESTINATION_TIME 3.0

#define UP_RIGHT_ANGLE 3.14159/4.0
#define UP_LEFT_ANGLE 3.0*3.14159/4.0
#define DOWN_LEFT_ANGLE 5.0*3.14159/4.0
#define DOWN_RIGHT_ANGLE 7.0*3.14159/4.0

/**
 * Constructor
 */
E_DiagoShooter::E_DiagoShooter(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	facing = DOWN;

	findFourDestinations();
	chooseBestDestination();
	lastDestinationTime = smh->getGameTime();
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
		//draws a line to the destinations
		//smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationUpLeft),smh->getScreenY(yDestinationUpLeft),ARGB(255,255,0,0));
		//smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationUpRight),smh->getScreenY(yDestinationUpRight),ARGB(255,255,255,0));
		//smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationDownLeft),smh->getScreenY(yDestinationDownLeft),ARGB(255,0,255,0));
		//smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationDownRight),smh->getScreenY(yDestinationDownRight),ARGB(255,0,0,255));

		smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(x+64*cos(3.14)),smh->getScreenY(y+64*sin(3.14)));
		smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(x+64*cos(UP_RIGHT_ANGLE)),smh->getScreenY(y+64*sin(UP_RIGHT_ANGLE)),ARGB(255,0,255,255));
		//renderNearbyTargetGrid();
	}
}

/**
 * Updates the enemy
 */
void E_DiagoShooter::update(float dt) {


	if (smh->timePassedSince(lastDestinationTime) >= destinationDuration) {
		lastDestinationTime = smh->getGameTime();
		//destinationDuration = smh->randomFloat(1.5,6.0);
		findFourDestinations();
		chooseBestDestination();
	}

	if (smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay) {
		bool canShoot = false;
		float shootAngle;

		if (canShootPlayer(UP_LEFT_ANGLE)) {
			canShoot = true;
			shootAngle = UP_LEFT_ANGLE;
		} else if (canShootPlayer(UP_RIGHT_ANGLE)) {
			canShoot = true;
			shootAngle = UP_RIGHT_ANGLE;
		} else if (canShootPlayer(DOWN_LEFT_ANGLE)) {
			canShoot = true;
			shootAngle = DOWN_LEFT_ANGLE;
		} else if (canShootPlayer(DOWN_RIGHT_ANGLE)) {
			canShoot = true;
			shootAngle = DOWN_RIGHT_ANGLE;
		}

		if (canShoot && smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay) {
			dx = dy = 0.0;
			setFacingPlayer();
			
			smh->projectileManager->addProjectile(x, y, projectileSpeed, shootAngle, projectileDamage,
					true,projectileHoming, rangedType, true);

			lastRangedAttack = smh->getGameTime();
		}
	}


	move(dt);
}

/*
 * findFourDestinations()
 *
 * For each of the 4 diagonals, finds the furthest point from Smiley that the Cone can get to,
 * and can hit Smiley from.
 * 
 * At the end of the function there will be 4 destination points created. If one of the points is (-1, -1), that
 * means a destination has not beed found for that direction.
 */ 
void E_DiagoShooter::findFourDestinations() {

	bool foundUpLeft, foundUpRight, foundDownLeft, foundDownRight;
	
	//set each of the directions to "false" and each of the destinations to -1. 
	foundUpLeft = foundUpRight = foundDownLeft = foundDownRight = false;
	xDestinationUpLeft = yDestinationUpLeft = xDestinationUpRight = yDestinationUpRight = xDestinationDownLeft = yDestinationDownLeft = xDestinationDownRight = yDestinationDownRight = -1;

	//loop from as far out as possible so that the cone avoids smiley.
	float maxDist = weaponRange * 0.707106781; //0.707 = cos(45 degrees)
	maxDist *= 0.95;
	
	if (maxDist > 6*64) maxDist = 6*64; //cap it at 6 tiles away -- that's far enough
	if (maxDist < 64) maxDist = 64;

	int n;
	int i,j;
	int gridI, gridJ;
	float shootAngle;
	
	for (n = maxDist; n > 0; n -= 10) {
		//up-left
		i = smh->player->x - n;
		j = smh->player->y - n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = DOWN_RIGHT_ANGLE;

		if (!foundUpLeft && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundUpLeft = true;
			xDestinationUpLeft = i; xGridDestUpLeft = gridI;
			yDestinationUpLeft = j; yGridDestUpLeft = gridJ;
		}

		//up-right
		i = smh->player->gridX + n;
		j = smh->player->gridY - n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = DOWN_LEFT_ANGLE;

		if (!foundUpRight && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundUpRight = true;
			xDestinationUpRight = i; xGridDestUpRight = gridI;
			yDestinationUpRight = j; yGridDestUpRight = gridJ;
		}

		//down-left
		i = smh->player->gridX - n;
		j = smh->player->gridY + n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = UP_RIGHT_ANGLE;

		if (!foundDownLeft && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundDownLeft = true;
			xDestinationDownLeft = i; xGridDestDownLeft = gridI;
			yDestinationDownLeft = j; yGridDestDownLeft = gridJ;
		}

		//down-right
		i = smh->player->gridX + n;
		j = smh->player->gridY + n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = UP_LEFT_ANGLE;

		if (!foundDownRight && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundDownRight = true;
			xDestinationDownRight = i; xGridDestDownRight = gridI;
			yDestinationDownRight = j; yGridDestDownRight = gridJ;
		}
	}

}

/*
 * chooseBestDestination
 *
 * Chooses the current destination out of the 4 diagonal destinations that have already been found
 * Chooses based on A* distance
 */
void E_DiagoShooter::chooseBestDestination() {
	bool hasDestination;
	int AStarToUpLeft=1000, AStarToUpRight=1000, AStarToDownLeft=1000, AStarToDownRight=1000;

	if (xGridDestUpLeft != -1) AStarToUpLeft = AStarDistance(xGridDestUpLeft, yGridDestUpLeft);
	if (xGridDestUpRight != -1) AStarToUpRight = AStarDistance(xGridDestUpRight, yGridDestUpRight);
	if (xGridDestDownLeft != -1) AStarToDownLeft = AStarDistance(xGridDestDownLeft, yGridDestDownLeft);
	if (xGridDestDownRight != -1) AStarToDownRight = AStarDistance(xGridDestDownRight, yGridDestDownRight);
	
	//find the minimum A* distance
	int minDist = min(AStarToUpLeft,AStarToUpRight);
	minDist = min(minDist,AStarToDownLeft);
	minDist = min(minDist,AStarToDownRight);

	     if (minDist == AStarToUpLeft) {xDestination = xDestinationUpLeft; yDestination = yDestinationUpLeft; xDestGrid = xGridDestUpLeft; yDestGrid = yGridDestUpLeft;}
	else if (minDist == AStarToUpRight) {xDestination = xDestinationUpRight; yDestination = yDestinationUpRight; xDestGrid = xGridDestUpRight; yDestGrid = yGridDestUpRight;}
	else if (minDist == AStarToDownLeft) {xDestination = xDestinationDownLeft; yDestination = yDestinationDownLeft; xDestGrid = xGridDestDownLeft; yDestGrid = yGridDestDownLeft;}
	else if (minDist == AStarToDownRight) {xDestination = xDestinationDownRight; yDestination = yDestinationDownRight; xDestGrid = xGridDestDownRight; yDestGrid = yGridDestDownRight;}

	if (minDist != 1000) {
		doAStar(xDestGrid, yDestGrid);
		hasDestination = true;
	} else {
		hasDestination = false;
	}
}