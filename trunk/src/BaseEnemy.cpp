/**
 * Implements concrete methods of the abstract BaseEnemy class.
 */
#include "SmileyEngine.h"
#include "EnemyFramework.h"
#include "Environment.h"
#include "Player.h"
#include "hgeresource.h"
#include "WeaponParticle.h"
#include "CollisionCircle.h"
#include "ProjectileManager.h"

extern SMH *smh;

#define ENEMY_FLASH_DURATION 0.5
#define ENEMY_FROZEN_DURATION 3.0

/**
 * Load enemy info from the enemy.dat file
 */
void BaseEnemy::initEnemy(int _id, int _gridX, int _gridY, int _groupID) {
		
	try
	{
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
		enemyType = smh->gameData->getEnemyInfo(id).enemyType;
		wanderType = smh->gameData->getEnemyInfo(id).wanderType;
		health = maxHealth = (float)smh->gameData->getEnemyInfo(id).hp / 100.0;
		damage = (float)smh->gameData->getEnemyInfo(id).damage / 100.0;
		radius = smh->gameData->getEnemyInfo(id).radius;
		speed = smh->gameData->getEnemyInfo(id).speed;
		immuneToTongue = smh->gameData->getEnemyInfo(id).immuneToTongue;
		immuneToFire = smh->gameData->getEnemyInfo(id).immuneToFire;
		immuneToStun = smh->gameData->getEnemyInfo(id).immuneToStun;
		immuneToLightning = smh->gameData->getEnemyInfo(id).immuneToLightning;
		chases = smh->gameData->getEnemyInfo(id).chases;
		variable1 = smh->gameData->getEnemyInfo(id).variable1;
		variable2 = smh->gameData->getEnemyInfo(id).variable2;
		variable3 = smh->gameData->getEnemyInfo(id).variable3;
		hasRangedAttack = smh->gameData->getEnemyInfo(id).hasRangedAttack;
		if (hasRangedAttack) {
			rangedType = smh->gameData->getEnemyInfo(id).rangedType;	
			weaponRange = smh->gameData->getEnemyInfo(id).range;
			rangedAttackDelay = (float)smh->gameData->getEnemyInfo(id).delay/(float)1000.0;
			projectileSpeed = smh->gameData->getEnemyInfo(id).projectileSpeed; //Here is a float-to-int conversion
			projectileDamage = smh->gameData->getEnemyInfo(id).projectileDamage;
			projectileYOffset = 0;
		}

		//Set pathing shit
		for (int i = 0; i < 256; i++) canPass[i] = false;
		canPass[WALKABLE] = smh->gameData->getEnemyInfo(id).land;
		canPass[SLIME] = smh->gameData->getEnemyInfo(id).slime;
		canPass[WALK_LAVA] = smh->gameData->getEnemyInfo(id).lava;
		canPass[DIZZY_MUSHROOM_1] = canPass[DIZZY_MUSHROOM_2] = smh->gameData->getEnemyInfo(id).mushrooms;
		canPass[SHALLOW_WATER] = smh->gameData->getEnemyInfo(id).shallowWater;
		canPass[SHALLOW_GREEN_WATER] = smh->gameData->getEnemyInfo(id).shallowWater;
		canPass[DEEP_WATER] = smh->gameData->getEnemyInfo(id).deepWater;
		canPass[GREEN_WATER] = smh->gameData->getEnemyInfo(id).deepWater;
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
		canPass[HOVER_PAD] = true;

		//Initialize stun star angles
		for (int i = 0; i < NUM_STUN_STARS; i++) {
			stunStarAngles[i] = (float)i * ((2.0*PI) / (float)NUM_STUN_STARS);
		}

		//Load graphics - each one has a 1 border transparent pixel layer around it except
		// for batlet caves because the graphics need to be put next to each other
		int borderSize = (smh->gameData->getEnemyInfo(id).enemyType == ENEMY_BATLET_DIST ? 0 : 1);
		int size = (smh->gameData->getEnemyInfo(id).enemyType == ENEMY_BATLET_DIST ? 64 : 62);
		graphic[LEFT] = new hgeAnimation(smh->resources->GetTexture("enemies"), 
			smh->gameData->getEnemyInfo(id).numFrames, 3, 
			smh->gameData->getEnemyInfo(id).gCol*64+borderSize, 
			smh->gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
		graphic[LEFT]->Play();
		graphic[RIGHT] = new hgeAnimation(smh->resources->GetTexture("enemies"), 
			smh->gameData->getEnemyInfo(id).numFrames, 3, 
			smh->gameData->getEnemyInfo(id).gCol*64 + 64 * (smh->gameData->getEnemyInfo(id).hasOneGraphic ? 0 : smh->gameData->getEnemyInfo(id).numFrames)+borderSize, 
			smh->gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
		graphic[RIGHT]->Play();
		graphic[UP] = new hgeAnimation(smh->resources->GetTexture("enemies"), 
			smh->gameData->getEnemyInfo(id).numFrames, 3, 
			smh->gameData->getEnemyInfo(id).gCol*64 + 2 * 64 * (smh->gameData->getEnemyInfo(id).hasOneGraphic ? 0 : smh->gameData->getEnemyInfo(id).numFrames)+borderSize, 
			smh->gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
		graphic[UP]->Play();
		graphic[DOWN] = new hgeAnimation(smh->resources->GetTexture("enemies"), 
			smh->gameData->getEnemyInfo(id).numFrames, 3, 
			smh->gameData->getEnemyInfo(id).gCol*64 + 3 * 64 * (smh->gameData->getEnemyInfo(id).hasOneGraphic ? 0 : smh->gameData->getEnemyInfo(id).numFrames)+borderSize,
			smh->gameData->getEnemyInfo(id).gRow*64+borderSize, size, size);
		graphic[DOWN]->Play();

		//Set graphic hot spots
		for (int i = 0; i < 4; i++) graphic[i]->SetHotSpot(32,32);

		//Set collision box
		collisionBox->SetRadius(x, y, radius);

	} 
	catch(System::Exception *ex) 
	{
		smh->hge->System_Log("-- Exception caught while initializing an enemy!");
		smh->hge->System_Log("EnemyID: %d, Location: (%d,%d), GroupID: %d", _id, _gridX, _gridY, _groupID);
		smh->hge->System_Log("Stack trace:");
		smh->hge->System_Log("%s", ex->ToString());
		smh->hge->System_Log("-- End Exception");
		throw ex;
	}
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
	float knockbackAngle = Util::getAngleBetween(knockbackerX, knockbackerY, x, y);
	
	knockbackXDist = knockbackDist * cos(knockbackAngle);
	knockbackYDist = knockbackDist * sin(knockbackAngle);
	
	knockbackTime = .2;
	knockback = true;
	startedKnockback = smh->getGameTime();

}

/**
 * Returns whether or not the enemy is within <range> squares of the player.
 */
bool BaseEnemy::inChaseRange(int range) {
	return (chases && mapPath[gridX][gridY] <= range && mapPath[gridX][gridY] > 0 && !smh->player->isInvisible() &&
		smh->environment->collision[smh->player->gridX][smh->player->gridY] != ENEMY_NO_WALK);
}

/**
 * Returns whether or not the enemy can hit the player with a projectile attack
 * at its current position.
 */
bool BaseEnemy::canShootPlayer() {
	return (distanceFromPlayer() <= weaponRange && smh->environment->validPath(x, y, smh->player->x, smh->player->y, smh->projectileManager->getProjectileRadius(rangedType), canPass));
}

bool BaseEnemy::canShootPlayer(float angle) {
	return (distanceFromPlayer() <= weaponRange && smh->environment->validPath(angle, x, y, smh->player->x, smh->player->y, smh->projectileManager->getProjectileRadius(rangedType), canPass, true));
}

bool BaseEnemy::canShootPlayer(float fromX, float fromY, float angle) {
	return (Util::distance(fromX,fromY,smh->player->x,smh->player->y) <= weaponRange && smh->environment->validPath(angle, fromX, fromY, smh->player->x, smh->player->y, smh->projectileManager->getProjectileRadius(rangedType), canPass, true));
}

/**
 * Returns the length of the straight line connecting the enemy to the
 * player's position.
 */
int BaseEnemy::distanceFromPlayer() {
	if (x == smh->player->x && y == smh->player->y) return 0;
	int xDist = abs(x - smh->player->x);
	int yDist = abs(y - smh->player->y);
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
		if (smh->timePassedSince(startedKnockback) > knockbackTime) knockback = false;
	} else {
		//Normal
		xDist = dx*dt;
		yDist = dy*dt;		
	}

	//Move left or right
	futureCollisionBox->SetRadius(x+xDist,y,28.0f);
	if (!smh->environment->enemyCollision(futureCollisionBox,this,dt)) {
		x += xDist;
	}

	//Move up or down
	futureCollisionBox->SetRadius(x,y+yDist,28.0f);
	if (!smh->environment->enemyCollision(futureCollisionBox,this,dt)) {
		y += yDist;
	}
}

/**
 * Calculate the mapPath array for an enemy
 */
void BaseEnemy::doAStar() {
	doAStar(smh->player->gridX, smh->player->gridY, 10);
}

void BaseEnemy::doAStar(int destinationX, int destinationY, int updateRadius) {

	boolean found;
	int lowValue;

	//Only update map path if the enemies are within 10 tiles of destination
	if (abs(gridX - destinationX) + abs(gridY - destinationY) > updateRadius) {
		return;
	}

	//For performance reasons only update mapPath in a *updateRadius* (default = 10) tile radius
	int startX = (gridX <= updateRadius) ? 0 : gridX - updateRadius;
	int startY = (gridY <= updateRadius) ? 0 : gridY - updateRadius;
	int endX = (gridX >= smh->environment->areaWidth - updateRadius) ? smh->environment->areaWidth : gridX + updateRadius;
	int endY = (gridY >= smh->environment->areaHeight - updateRadius) ? smh->environment->areaHeight : gridY + updateRadius;

	//Initialize mapPath array
	for (int i = startX; i < endX; i++) {
		for (int j = startY; j < endY; j++) {
			//If the player is at (i,j), the distance from (i,j) is 0
			if (i == destinationX && j == destinationY) {
				mapPath[i][j] = 0;
			//If (i,j) is inaccessible, set distance to 999
			} else if (!canPass[smh->environment->collision[i][j]] || smh->environment->hasSillyPad(i, j)) {			
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
							if (ni >= 0 && nj >= 0 && ni < smh->environment->areaWidth && nj <= smh->environment->areaHeight && !(ni == i && nj == j)) {
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
							if (ni >= 0 && nj >= 0 && ni < smh->environment->areaWidth && nj < smh->environment->areaHeight && !(ni == i && nj == j)) {
								//Verify this square hasn't been calculated already
								if (mapPath[ni][nj] >= 0) {
									
									//If diagonal, make sure you can actually get there
									bool diagonalOK=verifyDiagonal(i,j,ni,nj);
									
									//Assign the value to lowvalue if it is lower
									if (mapPath[ni][nj] < lowValue && diagonalOK) {
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
 * "Verify the diagonal"
 * This functional is used by the A* algorithm to make sure that the algorithm doesn't try to make enemies
   run "between" 2 diagonal objects. If they did try to run through, they'd be stuck!
 */
bool BaseEnemy::verifyDiagonal(int curX, int curY, int neighborX, int neighborY) {
	int diffX = neighborX - curX;
	int diffY = neighborY - curY;

	if (diffX == 0 || diffY == 0) return true; //Isn't diagonal, so don't worry about it.

	if (diffX != 0)  //is moving left or right, so check immediately left or right and return false if it's blocked
		if (!canPass[smh->environment->collision[neighborX][curY]] || mapPath[neighborX][curY] == 999) return false;

	if (diffY != 0) //is moving up or down, so check immediately up or down and return false if it's blocked
		if (!canPass[smh->environment->collision[curX][neighborY]] || mapPath[curX][neighborY] == 999) return false;

	//Passed both conditions above, so return true
	return true;
	
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
		int xDist = smh->player->x - x;
		int yDist = smh->player->y - y;
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
	timeStartedFlashing = smh->getGameTime();
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
	if (isSpawning) {
		//spawning, so update the spawn effect that until it's done
		if (spawnState == ENEMY_SPAWNSTATE_FALLING) {
			spawnY -= 100.0*dt;
			if (spawnY <= 0.0) {
				spawnY = 0.0;
				spawnState = ENEMY_SPAWNSTATE_GROWING;
			}
		} else if (spawnState == ENEMY_SPAWNSTATE_GROWING) {
			spawnSize += dt;
			if (spawnSize >= 1.0) {
				spawnSize = 1.0;
				isSpawning = false;
			}
		} else { //who knows how the state could be 'else', but oh well.
			isSpawning = false;
		}
		
		//Don't want to update anything until the spawn effect is done
		return;
	}

	//Update basic shit that every enemy needs
	collisionBox->SetRadius(x, y, radius);
	gridX = x / 64;
	gridY = y / 64;
	screenX = smh->getScreenX(x);
	screenY = smh->getScreenY(y);

	//Update statuses
	if (stunned && smh->timePassedSince(startedStun) > stunLength) {
		stunned = false;
	}
	if (flashing && smh->timePassedSince(timeStartedFlashing) > ENEMY_FLASH_DURATION) {
		flashing = false;
	}
	if (knockback && smh->timePassedSince(startedKnockback) > knockbackTime) {
		knockback = false;
	}
	if (frozen && smh->timePassedSince(timeFrozen) > ENEMY_FROZEN_DURATION) {
		frozen = false;
	}

	//Update default animations
	if (!frozen && !stunned && (abs(dx) > 0.0 || abs(dy) > 0.0)) {
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
		if (smh->player->fireBreathParticle->testCollision(collisionBox)) {
			frozen = false;
			dealDamageAndKnockback(smh->player->getFireBreathDamage()*dt, 
				500.0*dt,smh->player->x, smh->player->y);
		}
	}

}

/**
 * Performs cleanup required by every enemy. This should be called from each
 * enemy's destructor. It can't be done in a base destructor becuase C++
 * is a terrible, archaic language.
 */
void BaseEnemy::baseCleanup()
{
	if (currentState != NULL) delete currentState;
	if (collisionBox != NULL) delete collisionBox;
	if (futureCollisionBox != NULL) delete futureCollisionBox;
	
	for (int i = 0; i < 4; i++) 
	{
		delete graphic[i];
	}
}

/**
 * Draw method called by the EnemyManager every frame. This method contains
 * draw logic that must be done by every enemy in the framework and should never
 * need to be overridden!!!!
 */
void BaseEnemy::baseDraw(float dt) {
	if (isSpawning) {
		graphic[0]->RenderEx(smh->getScreenX(x),smh->getScreenY(y)-spawnY,0.0,spawnSize,spawnSize);
		//graphic[0]->Render(100,100);
		return;
	}

	//Call the enemy's draw function. If the enemy is currently flashing,
	//skip calling it some frames to create the flashing effect.
	if (!flashing || int(smh->getGameTime() * 100) % 10 > 5) {
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
	smh->resources->GetSprite("iceBlock")->Render(screenX, screenY);
}

/**
 * This is called right before the enemy dies in case they need to do
 * anything special.
 */
void BaseEnemy::notifyOfDeath()
{
}

/**
 * Draws the stunned graphic effect for the enemy. This is called automatically every
 * frame by the framework. If an enemy needs something more specific than the default
 * functionality then it should overwrite this method.
 */
void BaseEnemy::drawStunned(float dt) {
	for (int n = 0; n < NUM_STUN_STARS; n++) {
		stunStarAngles[n] += 2.0* PI * dt;
		smh->resources->GetSprite("stunStar")->Render(
		smh->getScreenX(x + cos(stunStarAngles[n])*25), 
		smh->getScreenY(y + sin(stunStarAngles[n])*7) - 30.0);
	}
}

/**
 * This draws symbols near the enemy to show what the enemy is immune to.
 * It is called by the EnemyManager method, drawEnemyImmunities. That is called if the player is currently selecting Clinton's Cane.
 */
void BaseEnemy::drawImmunities() {
	float xDraw=-30;
	float yDraw=30;

	//is invincible, so don't draw any immunities
	if (smh->gameData->getEnemyInfo(id).invincible) return;

	if (smh->gameData->getEnemyInfo(id).immuneToTongue) {
		smh->resources->GetAnimation("upgradeIcons")->SetFrame(2);
		smh->resources->GetAnimation("upgradeIcons")->RenderEx(smh->getScreenX(x+xDraw-8),smh->getScreenY(y+yDraw-15),0.0,0.67,0.67);
		xDraw += 30;
	}
	if (smh->gameData->getEnemyInfo(id).immuneToFire) {
		smh->resources->GetAnimation("abilities")->SetFrame(1);
		smh->resources->GetAnimation("abilities")->RenderEx(smh->getScreenX(x+xDraw),smh->getScreenY(y+yDraw),0.0,0.5,0.5);
		xDraw += 30;
	}
	if (smh->gameData->getEnemyInfo(id).immuneToStun) {
		smh->resources->GetAnimation("abilities")->SetFrame(2);
		smh->resources->GetAnimation("abilities")->RenderEx(smh->getScreenX(x+xDraw),smh->getScreenY(y+yDraw),0.0,0.5,0.5);
		xDraw += 30;
	}
	if (smh->gameData->getEnemyInfo(id).immuneToLightning) {
		smh->resources->GetAnimation("abilities")->SetFrame(4);
		smh->resources->GetAnimation("abilities")->RenderEx(smh->getScreenX(x+xDraw),smh->getScreenY(y+yDraw),0.0,0.5,0.5);
		xDraw += 30;
	}
	if (smh->gameData->getEnemyInfo(id).immuneToFreeze) {
		smh->resources->GetAnimation("abilities")->SetFrame(8);
		smh->resources->GetAnimation("abilities")->RenderEx(smh->getScreenX(x+xDraw),smh->getScreenY(y+yDraw),0.0,0.5,0.5);
		xDraw += 30;
	}
}
/**
 * Basic projectile collision functionality. Enemies can override this
 * for something more specific.
 */
void BaseEnemy::hitWithProjectile(int projectileType) {

}

/**
 * Notifies the enemy that it has been hit by Smiley's tongue. Enemies should
 * override this if they need some special functionality when hit by the tongue.
 */
void BaseEnemy::notifyTongueHit() {

}

/**
 * Basic tongue collision funcitonality. Enemies can override this
 * for something more specific. Returns true if the tongue hit
 * this enemy.
 */
bool BaseEnemy::doTongueCollision(Tongue *tongue, float damage) {
	
	//Check collision
	if (tongue->testCollision(collisionBox)) {
			
		//Make sure the enemy wasn't already hit by this attack
		if (smh->timePassedSince(lastHitByWeapon) > .5) {
			lastHitByWeapon = smh->getGameTime();
			dealDamageAndKnockback(damage, 65.0, smh->player->x, smh->player->y);
			startFlashing();
			return true;
		}

	}

	return false;
}

/**
 * Does default player collision using the collisionBox object. Enemies can
 * override this for something more specific.
 */ 
void BaseEnemy::doPlayerCollision() {
	if (dealsCollisionDamage && smh->player->collisionCircle->testBox(collisionBox)) {
		smh->player->dealDamageAndKnockback(damage, true, 115, x, y);
		std::string debugText;
		debugText = "baseenemy.cpp Smiley hit by enemy id " + Util::intToString(id) +
			//" at grid (" + Util::intToString(gridX) + "," + Util::intToString(gridY) +
			//") pos (" + Util::intToString((int)x) + "," + Util::intToString((int)y) + ")";
			" type " + Util::intToString((int)enemyType);
		smh->setDebugText(debugText);
	}
}

/**
 * Default debug draw. Draws the collisionBox object in red. Enemies can
 * override this for something more specific.
 */
void BaseEnemy::drawDebug() {
	if (smh->isDebugOn() && dealsCollisionDamage) {
		smh->drawCollisionBox(collisionBox, Colors::RED);
	}
}