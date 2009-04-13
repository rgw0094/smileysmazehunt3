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

#define DOWN_RIGHT_ANGLE 1.0*3.14159/4.0
#define DOWN_LEFT_ANGLE 3.0*3.14159/4.0
#define UP_LEFT_ANGLE 5.0*3.14159/4.0
#define UP_RIGHT_ANGLE 7.0*3.14159/4.0



/**
 * Constructor
 */
E_DiagoShooter::E_DiagoShooter(int id, int gridX, int gridY, int groupID) {

	//Call parent init method
	initEnemy(id, gridX, gridY, groupID);

	facing = DOWN;

	hasDestination=false;

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
		smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationUpLeft),smh->getScreenY(yDestinationUpLeft),ARGB(128,255,0,0));
		smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationUpRight),smh->getScreenY(yDestinationUpRight),ARGB(128,255,255,0));
		smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationDownLeft),smh->getScreenY(yDestinationDownLeft),ARGB(128,0,255,0));
		smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestinationDownRight),smh->getScreenY(yDestinationDownRight),ARGB(128,0,0,255));

		if (hasDestination) smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(xDestination),smh->getScreenY(yDestination),ARGB(255,0,255,255));
		else smh->hge->Gfx_RenderLine(smh->getScreenX(x),smh->getScreenY(y),smh->getScreenX(smh->player->x),smh->getScreenY(smh->player->y),ARGB(255,255,255,255));

		//renderDiagoAStarGrid();
		//renderBaseEnemyAStarGrid();
	}
}

/**
 * Updates the enemy
 */
void E_DiagoShooter::update(float dt) {


	if (smh->timePassedSince(lastDestinationTime) >= destinationDuration) {
		lastDestinationTime = smh->getGameTime();
		destinationDuration = smh->randomFloat(0.1,0.2);
		//destinationDuration = 1.5;
		findFourDestinations();
		chooseBestDestination();
	}

	bool canShoot = false;

	if (smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay) {
		
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
		} else {
			canShoot = false;
		}

		if (canShoot && smh->timePassedSince(lastRangedAttack) >= rangedAttackDelay) {
			
			setFacingPlayer();
			
			smh->projectileManager->addProjectile(x, y, projectileSpeed, shootAngle, projectileDamage,
					true,projectileHoming, rangedType, true);

			lastRangedAttack = smh->getGameTime();
		}
		
	}

	if (!canShoot) {
		moveDiago();
	}


	move(dt);
}

/*
 * moveDiago()
 *
 * moves the DiagoShooter based on the A*, or if there's a clear shot to the destination, goes straight there.
 */
void E_DiagoShooter::moveDiago() {
	
	if (!hasDestination) {
		//wander
		//setState(new ES_Wander(this));
	} else {
		dx = dy = 0;
		//Get out of wander state
		//setState(NULL);

		if (smh->environment->validPath(x,y,xDestination,yDestination,radius,canPass) || (gridX==xDestGrid && gridY==yDestGrid)) {
			if (x > xDestination + TARGET_ERROR) dx = -speed;
			if (x < xDestination - TARGET_ERROR) dx = speed;
			if (y > yDestination + TARGET_ERROR) dy = -speed;
			if (y < yDestination - TARGET_ERROR) dy = speed;
			//smh->setDebugText("Diago Using Destination");
		} else { //Use the A*
			
			//Choose a path towards the player
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
			if (targetX < gridX) dx = -speed;
			if (targetX > gridX) dx = speed;
			if (targetY < gridY) dy = -speed;
			if (targetY > gridY) dy = speed;
			//smh->setDebugText("Diago Using A*");

		}
		
		//Set facing direction
		if (abs(dy) > abs(dx)) {
			if (dy > 0) {
				facing = DOWN;
			} else if (dy < 0) {
				facing = UP;
			}
		} else {
			if (dx < 0) {
				facing = LEFT;
			} else if (dx > 0) {
				facing = RIGHT;
			}
		}

	
	}

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

	hasDestination = false;
	
	//set each of the directions to "false" and each of the destinations to -1. 
	foundUpLeft = foundUpRight = foundDownLeft = foundDownRight = false;
	xGridDestUpLeft = yGridDestUpLeft = xGridDestUpRight = yGridDestUpRight = xGridDestDownLeft = yGridDestDownLeft = xGridDestDownRight = yGridDestDownRight = -1;

	//loop from as far out as possible so that the cone avoids smiley.
	float maxDist = weaponRange * 0.707106781; //0.707 = cos(45 degrees)
	maxDist *= 0.95;
	
	if (maxDist > 6*64) maxDist = 6*64; //cap it at 6 tiles away -- that's far enough
	if (maxDist < 64) maxDist = 64;

	int n;
	int i,j;
	int gridI, gridJ;
	float shootAngle;
	int xOffset,yOffset;

	for (n = maxDist; n > 80; n -= 10) {
		
		//up-left
		i = smh->player->x - n;
		j = smh->player->y - n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = DOWN_RIGHT_ANGLE;
        xOffset = i - gridI*64; yOffset = j - gridJ*64;		

		if (!foundUpLeft && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundUpLeft = true;

			//It is possible that this point is non-ideal in that the cone cannot fit here (b/c it's hanging over the edge onto a collision square).
			//These next few lines try to fix that by setting found=false
			if (xOffset <= radius && smh->environment->isInBounds(gridI-1,gridJ) && !canPass[smh->environment->collision[gridI-1][gridJ]]) foundUpLeft = false;
			if (yOffset <= radius && smh->environment->isInBounds(gridI,gridJ-1) && !canPass[smh->environment->collision[gridI][gridJ-1]]) foundUpLeft = false;
			if (xOffset <= radius && yOffset <= radius && smh->environment->isInBounds(gridI-1,gridJ-1) && !canPass[smh->environment->collision[gridI-1][gridJ-1]]) foundUpLeft = false;
			
			if (foundUpLeft) {
				xDestinationUpLeft = i; xGridDestUpLeft = gridI;
				yDestinationUpLeft = j; yGridDestUpLeft = gridJ;
			}
		}

		//up-right
		i = smh->player->x + n;
		j = smh->player->y - n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = DOWN_LEFT_ANGLE;
		xOffset = i - gridI*64; yOffset = j - gridJ*64;

		if (!foundUpRight && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundUpRight = true;
			
			//It is possible that this point is non-ideal in that the cone cannot fit here (b/c it's hanging over the edge onto a collision square).
			//These next few lines try to fix that by setting found=false
			if (xOffset >= 64-radius && smh->environment->isInBounds(gridI+1,gridJ) && !canPass[smh->environment->collision[gridI+1][gridJ]]) foundUpRight = false;
			if (yOffset <= radius && smh->environment->isInBounds(gridI,gridJ-1) && !canPass[smh->environment->collision[gridI][gridJ-1]]) foundUpRight = false;
			if (xOffset >= 64-radius && yOffset <= radius && smh->environment->isInBounds(gridI+1,gridJ-1) && !canPass[smh->environment->collision[gridI+1][gridJ-1]]) foundUpRight = false;

			if (foundUpRight) {
				xDestinationUpRight = i; xGridDestUpRight = gridI;
				yDestinationUpRight = j; yGridDestUpRight = gridJ;
			}
		}

		//down-left
		i = smh->player->x - n;
		j = smh->player->y + n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = UP_RIGHT_ANGLE;
		xOffset = i - gridI*64; yOffset = j - gridJ*64;

		if (!foundDownLeft && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundDownLeft = true;
			
			//It is possible that this point is non-ideal in that the cone cannot fit here (b/c it's hanging over the edge onto a collision square).
			//These next few lines try to fix that by setting found=false
			if (xOffset <= radius && smh->environment->isInBounds(gridI-1,gridJ) && !canPass[smh->environment->collision[gridI-1][gridJ]]) foundDownLeft = false;
			if (yOffset >= 64-radius && smh->environment->isInBounds(gridI,gridJ+1) && !canPass[smh->environment->collision[gridI][gridJ+1]]) foundDownLeft = false;
			if (xOffset <= radius && yOffset >= 64-radius && smh->environment->isInBounds(gridI-1,gridJ+1) && !canPass[smh->environment->collision[gridI-1][gridJ+1]]) foundDownLeft = false;

			if (foundDownLeft) {
				xDestinationDownLeft = i; xGridDestDownLeft = gridI;
				yDestinationDownLeft = j; yGridDestDownLeft = gridJ;
			}
		}

		//down-right
		i = smh->player->x + n;
		j = smh->player->y + n;
		gridI = Util::getGridX(i);
		gridJ = Util::getGridY(j);
		shootAngle = UP_LEFT_ANGLE;
		xOffset = i - gridI*64; yOffset = j - gridJ*64;

		if (!foundDownRight && smh->environment->isInBounds(gridI,gridJ) && canPass[smh->environment->collision[gridI][gridJ]] && canShootPlayer(i,j,shootAngle)) {
			foundDownRight = true;

			//It is possible that this point is non-ideal in that the cone cannot fit here (b/c it's hanging over the edge onto a collision square).
			//These next few lines try to fix that by setting found=false
			if (xOffset >= 64-radius && smh->environment->isInBounds(gridI+1,gridJ) && !canPass[smh->environment->collision[gridI+1][gridJ]]) foundDownRight = false;
			if (yOffset >= 64-radius && smh->environment->isInBounds(gridI,gridJ+1) && !canPass[smh->environment->collision[gridI][gridJ+1]]) foundDownRight = false;
			if (xOffset >= 64-radius && yOffset >= 64-radius && smh->environment->isInBounds(gridI+1,gridJ+1) && !canPass[smh->environment->collision[gridI+1][gridJ+1]]) foundDownRight = false;

			if (foundDownRight) {
				xDestinationDownRight = i; xGridDestDownRight = gridI;
				yDestinationDownRight = j; yGridDestDownRight = gridJ;
			}
		}
	}

	std::string debugText;
	debugText = "UL" + Util::intToString(foundUpLeft) + "; UR" + Util::intToString(foundUpRight) + "; DL" + Util::intToString(foundDownLeft) + "; DR" + Util::intToString(foundDownRight);
	//smh->setDebugText(debugText);

}

/*
 * chooseBestDestination
 *
 * Chooses the current destination out of the 4 diagonal destinations that have already been found
 * Chooses based on A* distance
 */
void E_DiagoShooter::chooseBestDestination() {
	hasDestination=false;
	
	AStarFromDiago();

	int AStarToUpLeft=1000, AStarToUpRight=1000, AStarToDownLeft=1000, AStarToDownRight=1000;

	int xGrid, yGrid;

	//get the A* distance for upleft, upright, downleft, and downright
	if (xGridDestUpLeft != -1) {
		xGrid = xGridDestUpLeft - gridX + xAStarGridOffset;
		yGrid = yGridDestUpLeft - gridY + yAStarGridOffset;
		if (xGrid >= 0 && xGrid <= AStarGridSize-1 && yGrid >= 0 && yGrid <= AStarGridSize-1)
			AStarToUpLeft = AStarGrid[xGrid][yGrid];
	}
	if (xGridDestUpRight != -1) {
		xGrid = xGridDestUpRight - gridX + xAStarGridOffset; 
		yGrid = yGridDestUpRight - gridY + yAStarGridOffset;
		if (xGrid >= 0 && xGrid <= AStarGridSize-1 && yGrid >= 0 && yGrid <= AStarGridSize-1)
			AStarToUpRight = AStarGrid[xGrid][yGrid];
	}
	if (xGridDestDownLeft != -1) {
		xGrid = xGridDestDownLeft - gridX + xAStarGridOffset;
		yGrid = yGridDestDownLeft - gridY + yAStarGridOffset;
		if (xGrid >= 0 && xGrid <= AStarGridSize-1 && yGrid >= 0 && yGrid <= AStarGridSize-1)
			AStarToDownLeft = AStarGrid[xGrid][yGrid];	
	}
	if (xGridDestDownRight != -1) {
		xGrid = xGridDestDownRight - gridX + xAStarGridOffset;
		yGrid = yGridDestDownRight - gridY + yAStarGridOffset;
		if (xGrid >= 0 && xGrid <= AStarGridSize-1 && yGrid >= 0 && yGrid <= AStarGridSize-1)
			AStarToDownRight = AStarGrid[xGrid][yGrid];
	}
	
	//find the minimum A* distance
	int minDist;
	minDist = min(AStarToUpLeft,AStarToUpRight);
	minDist = min(minDist,AStarToDownLeft);
	minDist = min(minDist,AStarToDownRight);

	     if (minDist == AStarToUpLeft) {xDestination = xDestinationUpLeft; yDestination = yDestinationUpLeft; xDestGrid = xGridDestUpLeft; yDestGrid = yGridDestUpLeft;}
	else if (minDist == AStarToUpRight) {xDestination = xDestinationUpRight; yDestination = yDestinationUpRight; xDestGrid = xGridDestUpRight; yDestGrid = yGridDestUpRight;}
	else if (minDist == AStarToDownLeft) {xDestination = xDestinationDownLeft; yDestination = yDestinationDownLeft; xDestGrid = xGridDestDownLeft; yDestGrid = yGridDestDownLeft;}
	else if (minDist == AStarToDownRight) {xDestination = xDestinationDownRight; yDestination = yDestinationDownRight; xDestGrid = xGridDestDownRight; yDestGrid = yGridDestDownRight;}

	std::string debugText;
	if (minDist != 1000) {		
		doAStar(xDestGrid, yDestGrid, 16);
		hasDestination = true;
		debugText = "true ";
	} else {
		hasDestination = false;
		debugText = "false ";
	}

	
	debugText += "UpLeft" + Util::intToString(AStarToUpLeft) + "; UpRight" + Util::intToString(AStarToUpRight) +
		"; DownLeft" + Util::intToString(AStarToDownLeft) + "; DownRight" + Util::intToString(AStarToDownRight) + "; minDist" + Util::intToString(minDist);
	if (distanceFromPlayer() < 600) smh->setDebugText(debugText);
}

void E_DiagoShooter::AStarFromDiago() {
	int i,j;
	xAStarGridOffset = 7; //where the DiagoShooter is in the 11x11 A* grid
	yAStarGridOffset = 7;
	AStarGridSize = 15;
    	
	//first reset the A* grid
	for (i=0; i<AStarGridSize; i++) {
		for (j=0; j<AStarGridSize; j++) {
			AStarGrid[i][j] = 1000;
			AStarBool[i][j] = false;
		}
	}
	AStarGrid[xAStarGridOffset][yAStarGridOffset] = 0;

	createAStarGrid(xAStarGridOffset,yAStarGridOffset);
}

void E_DiagoShooter::createAStarGrid(int i, int j) {
	
	int curNum = AStarGrid[i][j];
	int mapI = i + gridX - xAStarGridOffset; //At i == xAStarGridOffset, mapI = gridX
	int mapJ = j + gridY - yAStarGridOffset; //At j == yAStarGridOffset, mapJ = gridY

	//left
	if (i > 0 && smh->environment->isInBounds(mapI-1,mapJ)) {
		if (canPass[smh->environment->collision[mapI-1][mapJ]] && !smh->environment->hasSillyPad(mapI-1,mapJ)) {
			if (curNum+1 < AStarGrid[i-1][j]) {
				AStarGrid[i-1][j] = curNum + 1;
				createAStarGrid(i-1,j);
			}
		}
	}

	//right
	if (i < AStarGridSize-1 && smh->environment->isInBounds(mapI+1,mapJ)) {
		if (canPass[smh->environment->collision[mapI+1][mapJ]] && !smh->environment->hasSillyPad(mapI+1,mapJ)) {
			if (curNum+1 < AStarGrid[i+1][j]) {
				AStarGrid[i+1][j] = curNum + 1;
				createAStarGrid(i+1,j);
			}
		}
	}

	//up
	if (j > 0 && smh->environment->isInBounds(mapI,mapJ-1)) {
		if (canPass[smh->environment->collision[mapI][mapJ-1]] && !smh->environment->hasSillyPad(mapI,mapJ-1)) {
			if (curNum+1 < AStarGrid[i][j-1]) {
				AStarGrid[i][j-1] = curNum + 1;
				createAStarGrid(i,j-1);
			}
		}
	}

	//down
	if (j < AStarGridSize && smh->environment->isInBounds(mapI,mapJ+1)) {
		if (canPass[smh->environment->collision[mapI][mapJ+1]] && !smh->environment->hasSillyPad(mapI,mapJ+1)) {
			if (curNum+1 < AStarGrid[i][j+1]) {
				AStarGrid[i][j+1] = curNum + 1;
				createAStarGrid(i,j+1);
			}
		}
	}
}

void E_DiagoShooter::renderBaseEnemyAStarGrid() {
	for (int i = gridX - 8; i <= gridX + 8; i++) {
		for (int j = gridY - 8; j <= gridY + 8; j++) {
			if (smh->environment->isInBounds(i,j)) {
				smh->resources->GetFont("numberFnt")->printf(smh->getScreenX(i*64+32),smh->getScreenY(j*64+32),HGETEXT_CENTER,"%d",mapPath[i][j]);
			}
		}
	}

}

void E_DiagoShooter::renderDiagoAStarGrid() {
	int xAStarGrid, yAStarGrid;
	std::string dickens;

	for (int i = gridX - 7; i <= gridX + 7; i++) {
		for (int j = gridY - 7; j <= gridY + 7; j++) {
			if (smh->environment->isInBounds(i,j)) {
				xAStarGrid = i - gridX + xAStarGridOffset; if (xAStarGrid < 0) xAStarGrid = 0; if (xAStarGrid > AStarGridSize-1) xAStarGrid = AStarGridSize-1;
				yAStarGrid = j - gridY + yAStarGridOffset; if (yAStarGrid < 0) yAStarGrid = 0; if (yAStarGrid > AStarGridSize-1) yAStarGrid = AStarGridSize-1;
				dickens = Util::intToString(AStarGrid[xAStarGrid][yAStarGrid]);
				smh->resources->GetFont("curlz")->printf(smh->getScreenX(i*64+32),smh->getScreenY(j*64+32),HGETEXT_CENTER,dickens.c_str());
				if (i == targetX && j == targetY) smh->resources->GetFont("curlz")->printf(smh->getScreenX(i*64+32),smh->getScreenY(j*64+32),HGETEXT_CENTER,"*");
			}
		}
	}
}
