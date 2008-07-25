/**
 * Implements concrete methods of the abstract BaseEnemy class.
 */
#include "enemy.h"
#include "Environment.h"
#include "Player.h"
#include "hgeresource.h"
#include "hge.h"
#include "GameData.h"
#include "WeaponParticle.h"
#include "CollisionCircle.h"
#include "Tongue.h"
#include "EnemyState.h"
#include "EnemyManager.h"

extern Player *thePlayer;
extern Environment *theEnvironment;
extern GameData *gameData;
extern hgeResourceManager *resources;
extern HGE *hge;
extern float gameTime;
extern bool debugMode;


#define ENEMY_FLASH_DURATION 0.5
#define ENEMY_FROZEN_DURATION 3.0

/**
 * Load enemy info from the enemy.dat file
 */
void BaseEnemy::initEnemy(int _id, int _gridX, int _gridY, int _groupID) {
		
	currentState = NULL;

	//Set basic values
	id = _id;
	groupID = _groupID;
	gridX = _gridX;
	gridY = _gridY;
	startX = gridX;
	startY = gridY;
	x = gridX * 64 + 32;
	y = gridY * 64 + 32;
	collisionBox = new hgeRect();
	futureCollisionBox = new hgeRect();
	lastHitByWeapon = -100.0;
	facing = DOWN;
	frozen = false;
	stunned = false;
	dying = false;
	dealsCollisionDamage = true;
	flashing = false;

	//Load enemy info
	enemyType = gameData->getEnemyInfo(id).enemyType;
	wanderType = gameData->getEnemyInfo(id).wanderType;
	health = maxHealth = (float)gameData->getEnemyInfo(id).hp / 100.0;
	damage = (float)gameData->getEnemyInfo(id).damage / 100.0;
	radius = gameData->getEnemyInfo(id).radius;
	speed = gameData->getEnemyInfo(id).speed;
	immuneToTongue = gameData->getEnemyInfo(id).immuneToTongue;
	immuneToFire = gameData->getEnemyInfo(id).immuneToFire;
	immuneToStun = gameData->getEnemyInfo(id).immuneToStun;
	immuneToLightning = gameData->getEnemyInfo(id).immuneToLightning;
	chases = gameData->getEnemyInfo(id).chases;
	variable1 = gameData->getEnemyInfo(id).variable1;
	variable2 = gameData->getEnemyInfo(id).variable2;
	hasRangedAttack = gameData->getEnemyInfo(id).hasRangedAttack;
	if (hasRangedAttack) {
		rangedType = gameData->getEnemyInfo(id).rangedType;	
		weaponRange = gameData->getEnemyInfo(id).range;
		rangedAttackDelay = gameData->getEnemyInfo(id).delay;
		projectileSpeed = gameData->getEnemyInfo(id).projectileSpeed; //Here is a float-to-int conversion
		projectileDamage = gameData->getEnemyInfo(id).projectileDamage;
	}

	//Set pathing shit
	for (int i = 0; i < 256; i++) canPass[i] = false;
	canPass[WALKABLE] = gameData->getEnemyInfo(id).land;
	canPass[SLIME] = gameData->getEnemyInfo(id).slime;
	canPass[WALK_LAVA] = gameData->getEnemyInfo(id).lava;
	canPass[DIZZY_MUSHROOM_1] = canPass[DIZZY_MUSHROOM_2] = gameData->getEnemyInfo(id).mushrooms;
	canPass[SHALLOW_WATER] = gameData->getEnemyInfo(id).shallowWater;
	canPass[DEEP_WATER] = gameData->getEnemyInfo(id).deepWater;
	canPass[GREEN_WATER] = gameData->getEnemyInfo(id).deepWater;
	canPass[LEFT_ARROW] = true;
	canPass[RIGHT_ARROW] = true;
	canPass[UP_ARROW] = true;
	canPass[DOWN_ARROW] = true;
	canPass[PLAYER_START] = true;
	canPass[PLAYER_END] = true;
	canPass[WHITE_CYLINDER_DOWN] = true;
	canPass[YELLOW_CYLINDER_DOWN] = true;
	canPass[GREEN_CYLINDER_DOWN] = true;
	canPass[BLUE_CYLINDER_DOWN] = true;
	canPass[BROWN_CYLINDER_DOWN] = true;
	canPass[SILVER_CYLINDER_DOWN] = true;

	//Initialize stun star angles
	for (int i = 0; i < NUM_STUN_STARS; i++) {
		stunStarAngles[i] = (float)i * ((2.0*PI) / (float)NUM_STUN_STARS);
	}

	//Load graphics - each one has a 1 border transparent pixel layer around it except
	// for batlet caves because the graphics need to be put next to each other
	int borderSize = (gameData->getEnemyInfo(id).enemyType == ENEMY_BATLET_DIST ? 0 : 1);
	int size = (gameData->getEnemyInfo(id).enemyType == ENEMY_BATLET_DIST ? 64 : 62);
	graphic[LEFT] = new hgeAnimation(resources->GetTexture("enemies"), 
		gameData->getEnemyInfo(id).numFrames, 3, 
		gameData->getEnemyInfo(id).gCol*64+borderSize, 
		gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
	graphic[LEFT]->Play();
	graphic[RIGHT] = new hgeAnimation(resources->GetTexture("enemies"), 
		gameData->getEnemyInfo(id).numFrames, 3, 
		gameData->getEnemyInfo(id).gCol*64 + 64 * gameData->getEnemyInfo(id).numFrames+borderSize, 
		gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
	graphic[RIGHT]->Play();
	graphic[UP] = new hgeAnimation(resources->GetTexture("enemies"), 
		gameData->getEnemyInfo(id).numFrames, 3, 
		gameData->getEnemyInfo(id).gCol*64 + 2 * 64 * gameData->getEnemyInfo(id).numFrames+borderSize, 
		gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
	graphic[UP]->Play();
	graphic[DOWN] = new hgeAnimation(resources->GetTexture("enemies"), 
		gameData->getEnemyInfo(id).numFrames, 3, 
		gameData->getEnemyInfo(id).gCol*64 + 3 * 64 * gameData->getEnemyInfo(id).numFrames+borderSize, 
		gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
	graphic[DOWN]->Play();

	//Set graphic hot spots
	for (int i = 0; i < 4; i++) graphic[i]->SetHotSpot(32,32);

	//Set collision box
	collisionBox->SetRadius(x, y, radius);

}


/**
 * Switches states and calls exitState() on the old state and enterState() 
 * on the new.
 */
void BaseEnemy::setState(EnemyState *newState) {

	//Exit old state
	if (currentState) {
		currentState->exitState();
		delete currentState;
	}

	//Enter new state
	currentState = newState;
	if (currentState) {
		currentState->enterState();
	}

}

/**
 * Deals the specified damage to the enemy and knocks it back.
 *
 * @param damage		Damage to deal
 * @param knockbackDist Distance to knock smiley back from the center of
 *						The knockbacker
 * @param knockbackerX  x location of the object that knocked smiley back
 * @param knockbackerY  y location of the object that knocked smiley back
 */ 
void BaseEnemy::dealDamageAndKnockback(float damage, float knockbackDist,
									float knockbackerX, float knockbackerY) {

	health -= damage;
	float knockbackAngle = getAngleBetween(knockbackerX, knockbackerY, x, y);
	
	knockbackXDist = knockbackDist * cos(knockbackAngle);
	knockbackYDist = knockbackDist * sin(knockbackAngle);
	
	knockbackTime = .2;
	knockback = true;
	startedKnockback = gameTime;

}

/**
 * Returns whether or not the enemy is within <range> squares of the player.
 */
bool BaseEnemy::inChaseRange(int range) {
	return (chases && mapPath[gridX][gridY] <= range && mapPath[gridX][gridY] > 0 && !thePlayer->isInvisible() &&
		theEnvironment->collision[thePlayer->gridX][thePlayer->gridY] != ENEMY_NO_WALK);
}

/**
 * Returns whether or not the enemy can hit the player with a projectile attack
 * at its current position.
 */
bool BaseEnemy::canShootPlayer() {
	return (distanceFromPlayer() <= weaponRange && theEnvironment->validPath(x, y, thePlayer->x, thePlayer->y, 26, canPass));
}

/**
 * Returns the length of the straight line connecting the enemy to the
 * player's position.
 */
int BaseEnemy::distanceFromPlayer() {
	if (x == thePlayer->x && y == thePlayer->y) return 0;
	int xDist = abs(x - thePlayer->x);
	int yDist = abs(y - thePlayer->y);
	return sqrt(float(xDist*xDist) + float(yDist*yDist));
}


/**
 * Handles moving the enemy for this frame - checks collision and shit
 */ 
void BaseEnemy::move(float dt) {

	if (stunned || frozen) return;

	//Determine how far the enemy should move this frame based on dx,dy and 
	//knockback status
	float xDist, yDist;
	if (knockback) {
		//Knockback
		xDist = dx*dt + knockbackXDist / knockbackTime*dt;
		yDist = dy*dt + knockbackYDist / knockbackTime*dt;
		if (timePassedSince(startedKnockback) > knockbackTime) knockback = false;
	} else {
		//Normal
		xDist = dx*dt;
		yDist = dy*dt;		
	}

	//Move left or right
	futureCollisionBox->SetRadius(x+xDist,y,28.0f);
	if (!theEnvironment->enemyCollision(futureCollisionBox,this,dt)) {
		x += xDist;
	}

	//Move up or down
	futureCollisionBox->SetRadius(x,y+yDist,28.0f);
	if (!theEnvironment->enemyCollision(futureCollisionBox,this,dt)) {
		y += yDist;
	}
}

/**
 * Calculate the mapPath array for an enemy
 */
void BaseEnemy::doAStar() {

	boolean found;
	int lowValue;

	//Only update map path if the enemies are within 10 tiles of smiley
	if (abs(gridX - thePlayer->gridX) + abs(gridY - thePlayer->gridY) > 10) {
		return;
	}

	//For performance reasons only update mapPath in a 10 tile radius
	int startX = (gridX <= 10) ? 0 : gridX - 10;
	int startY = (gridY <= 10) ? 0 : gridY - 10;
	int endX = (gridX >= theEnvironment->areaWidth - 10) ? theEnvironment->areaWidth : gridX + 10;
	int endY = (gridY >= theEnvironment->areaHeight - 10) ? theEnvironment->areaHeight : gridY + 10;

	//Initialize mapPath array
	for (int i = startX; i < endX; i++) {
		for (int j = startY; j < endY; j++) {
			//If the player is at (i,j), the distance from (i,j) is 0
			if (i == thePlayer->gridX && j == thePlayer->gridY) {
				mapPath[i][j] = 0;
			//If (i,j) is inaccessible, set distance to 999
			} else if (!canPass[theEnvironment->collision[i][j]] || theEnvironment->hasSillyPad[i][j]) {			
				mapPath[i][j] = 999;
			//Otherwise put a -1
			} else {
				mapPath[i][j] = -1;
			}
		}
	}

	//Loop until no new positions are calculated
	do {
		found = false;		//no position has been calculated yet
		
		//First check for tiles which can be mapped. Set these to markMap == true.
		for (int i = startX; i < endX; i++) {
			for (int j = startY; j < endY; j++) {	
				markMap[i][j] = false;
				//If (i,j) hasn't been calculated yet
				if (mapPath[i][j] == -1) {
					//Scan neighbors
					for (int ni = i-1; ni <= i+1; ni++) {
						for (int nj = j-1; nj <= j+1; nj++) {
							//Verify neighbor is on map and (ni,nj) != (i,j)
							if (ni >= 0 && nj >= 0 && ni < theEnvironment->areaWidth && nj <= theEnvironment->areaHeight && !(ni == i && nj == j)) {
								//Verify square hasn't been calculated yet and is accessible to this enemy
								if (mapPath[ni][nj] >= 0 && mapPath[ni][nj] != 999) {
									markMap[i][j] = true;	//this cell can be calculated
									found = true;			//a calculatable cell has been found
								}
							}
						}
					}
				}
			}
		}


		//Next scan all marked squares and calculate the distance.
		for (int i = startX; i < endX; i++) {
			for (int j = startY; j < endY; j++) {
				if (markMap[i][j]) {
					lowValue = 999;
					//Loop through neighbors
					for (int ni = i-1; ni <= i+1; ni++) {
						for (int nj = j-1; nj <= j+1; nj++) {
							//Verify the neighbor is on the map
							if (ni >= 0 && nj >= 0 && ni < theEnvironment->areaWidth && nj < theEnvironment->areaHeight && !(ni == i && nj == j)) {
								//Verify this square hasn't been calculated already
								if (mapPath[ni][nj] >= 0) {
									//Assign the value to lowvalue if it is lower
									if (mapPath[ni][nj] < lowValue) {
										lowValue = mapPath[ni][nj];
									}
								}
							}
						}
					}
					//Assign the lowest neighbor value +1 to the square
					mapPath[i][j] = lowValue+1;
				}
			}
		}

	} while (found);
}

/**
 * Sets the enemy to face the player no matter how far away he is.
 */
void BaseEnemy::setFacingPlayer() {
	setFacingPlayer(999999.0, DOWN);
}

/**
 * Sets the enemy to face the player if the player is within a certain 
 * distance.
 *
 * @param maximumDistance	If the player is more than this distance away
 *							from the enemy then it will not change direction
 * @param defaultDirection	Direction to face if the player is more than
 *							maximumDistance away, or -1 to stay the current
 *							direction.
 */
void BaseEnemy::setFacingPlayer(int maximumDistance, int defaultDirection) {

	if (distanceFromPlayer() < maximumDistance) {
		int xDist = thePlayer->x - x;
		int yDist = thePlayer->y - y;
		if (xDist < 0 && yDist < 0) {
			//Player up-left from enemy
			if (abs(xDist) > abs(yDist)) {
				facing = LEFT;
			} else {
				facing = UP;
			}
		} else if (xDist > 0 && yDist < 0) {
			//Player up-right from enemy
			if (abs(xDist) > abs(yDist)) {
				facing = RIGHT;
			} else {
				facing = UP;
			}
		} else if (xDist < 0 && yDist > 0) {
			//Player  left-down from enemy
			if (abs(xDist) > abs(yDist)) {
				facing = LEFT;
			} else {
				facing = DOWN;
			}
		} else if (xDist > 0 && yDist > 0) {
			//Player right-down from enemy
			if (abs(xDist) > abs(yDist)) {
				facing = RIGHT;	
			} else {
				facing = DOWN;
			}
		}
	} else {
		facing = defaultDirection;
	}
}

/** 
 * Make the enemy flash after being hit.
 */ 
void BaseEnemy::startFlashing() {
	flashing = true;
	timeStartedFlashing = gameTime;
}

/**
 * Sets the facing direction of the enemy based on its current dx/dy
 */
void BaseEnemy::setFacing() {

	if (abs(dx) > abs(dy)) {
		if (dx > 0) facing = RIGHT;
		if (dx < 0) facing = LEFT;
	} else {
		if (dy > 0) facing = DOWN;
		if (dy < 0) facing = UP;
	}

}

/**
 * Update method called by the EnemyManager every frame. This method contains
 * things that need to be done by every enemy in the framework and should never
 * need to be overrode!!!!
 */
void BaseEnemy::baseUpdate(float dt) {

	//Update basic shit that every enemy needs
	collisionBox->SetRadius(x, y, radius);
	gridX = x / 64;
	gridY = y / 64;
	screenX = x - theEnvironment->xGridOffset*64 - theEnvironment->xOffset; 
	screenY = y - theEnvironment->yGridOffset*64 - theEnvironment->yOffset;

	//Update statuses
	if (stunned && timePassedSince(startedStun) > stunLength) {
		stunned = false;
	}
	if (flashing && timePassedSince(timeStartedFlashing) > ENEMY_FLASH_DURATION) {
		flashing = false;
	}
	if (knockback && timePassedSince(startedKnockback) > knockbackTime) {
		knockback = false;
	}
	if (frozen && timePassedSince(timeFrozen) > ENEMY_FROZEN_DURATION) {
		frozen = false;
	}

	//Update default animations
	if (!frozen && !stunned) {
		for (int n = 0; n < 4; n++) {
			graphic[n]->Update(dt);
		}
	}

	//Update the enemy's current state
	if (currentState) {
		currentState->update(dt);
	}

	//Call the enemy's update method
	update(dt);
		
	//Do player collision
	doPlayerCollision();

	//Fire breath collision
	if (!immuneToFire) {
		if (thePlayer->fireBreathParticle->testCollision(collisionBox)) {
			frozen = false;
			dealDamageAndKnockback(thePlayer->getFireBreathDamage()*dt, 
				500.0*dt,thePlayer->x, thePlayer->y);
		}
	}

}

/**
 * Draw method called by the EnemyManager every frame. This method contains
 * draw logic that must be done by every enemy in the framework and should never
 * need to be overrode!!!!
 */
void BaseEnemy::baseDraw(float dt) {

	//Call the enemy's draw function. If the enemy is currently flashing,
	//skip calling it some frames to create the flashing effect.
	if (!flashing || int(gameTime * 100) % 10 > 5) {
		draw(dt);
	}

	//Draw the ice block over the enemy if its frozen
	if (frozen) {
		drawFrozen(dt);
	}

	//Stunned enemy
	if (stunned) {
		drawStunned(dt);
	}

	//Debug mode stuff
	drawDebug();
}

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Virtual Methods ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////


/**
 * Draws the frozen graphic effect for the enemy. This is called automatically every
 * frame by the framework. If an enemy needs something more specific than the default
 * functionality then it should overwrite this method.
 */
void BaseEnemy::drawFrozen(float dt) {
	resources->GetSprite("iceBlock")->Render(screenX, screenY);
}

/**
 * Draws the stunned graphic effect for the enemy. This is called automatically every
 * frame by the framework. If an enemy needs something more specific than the default
 * functionality then it should overwrite this method.
 */
void BaseEnemy::drawStunned(float dt) {
	for (int n = 0; n < NUM_STUN_STARS; n++) {
		stunStarAngles[n] += 2.0* PI * dt;
		resources->GetSprite("stunStar")->Render(
		getScreenX(x + cos(stunStarAngles[n])*25), 
		getScreenY(y + sin(stunStarAngles[n])*7) - 30.0);
	}
}

/**
 * Basic projectile collision functionality. Enemies can override this
 * for something more specific.
 */
void BaseEnemy::hitWithProjectile(int projectileType) {

}

/**
 * Basic tongue collision funcitonality. Enemies can override this
 * for something more specific.
 */
void BaseEnemy::doTongueCollision(Tongue *tongue, float damage) {
	
	//Check collision
	if (tongue->testCollision(collisionBox)) {
			
		//Make sure the enemy wasn't already hit by this attack
		if (timePassedSince(lastHitByWeapon) > .5) {
			
			lastHitByWeapon = gameTime;
			dealDamageAndKnockback(damage, 65.0, thePlayer->x, thePlayer->y);
			startFlashing();
		}

	}
}

/**
 * Does default player collision using the collisionBox object. Enemies can
 * override this for something more specific.
 */ 
void BaseEnemy::doPlayerCollision() {
	if (dealsCollisionDamage && thePlayer->collisionCircle->testBox(collisionBox)) {
		thePlayer->dealDamageAndKnockback(damage, true, 115, x, y);
	}
}

/**
 * Default debug draw. Draws the collisionBox object in red. Enemies can
 * override this for something more specific.
 */
void BaseEnemy::drawDebug() {
	if (debugMode && dealsCollisionDamage) {
		drawCollisionBox(collisionBox, RED);
	}
}